#include "machine/rtc.hpp"

namespace ai386 {
namespace {

constexpr std::uint64_t kNanosPerSecond = 1000000000ULL;
constexpr std::uint64_t kQuartzHz = 32768;
constexpr std::uint8_t kStatusBSet = 0x80;
constexpr std::uint8_t kStatusBPie = 0x40;
constexpr std::uint8_t kStatusBAie = 0x20;
constexpr std::uint8_t kStatusBUie = 0x10;
constexpr std::uint8_t kStatusBDm = 0x04;
constexpr std::uint8_t kStatusB24Hour = 0x02;
constexpr std::uint8_t kStatusCIrqf = 0x80;
constexpr std::uint8_t kStatusCPf = 0x40;
constexpr std::uint8_t kStatusCAf = 0x20;
constexpr std::uint8_t kStatusCUf = 0x10;
constexpr std::uint8_t kStatusAUip = 0x80;

// Periodic interrupt rates for RS = 0..15 (MC146818 table), Hz.
constexpr int kPeriodicRates[16] = {0,   256, 128, 8192, 4096, 2048, 1024, 512,
                                    256, 128, 64,  32,   16,   8,    4,    2};

// Days elapsed since 1970-01-01 for a civil date (Howard Hinnant algorithm).
std::int64_t DaysFromCivil(int year, int month, int day) {
  const std::int64_t y = year - (month <= 2 ? 1 : 0);
  const std::int64_t era = (y >= 0 ? y : y - 399) / 400;
  const std::int64_t yoe = y - era * 400;
  const std::int64_t doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const std::int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;
}

void CivilFromDays(std::int64_t days, int& year, int& month, int& day) {
  days += 719468;
  const std::int64_t era = (days >= 0 ? days : days - 146096) / 146097;
  const std::int64_t doe = days - era * 146097;
  const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const std::int64_t y = yoe + era * 400;
  const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const std::int64_t mp = (5 * doy + 2) / 153;
  day = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
  month = static_cast<int>(mp + (mp < 10 ? 3 : -9));
  year = static_cast<int>(y + (month <= 2 ? 1 : 0));
}

}  // namespace

Rtc::Rtc(Pic& pic, std::int64_t start_epoch) : pic_(pic), epoch_(start_epoch) {
  regs_[kRegStatusD] = 0x80;  // VRT: battery good
}

void Rtc::Advance(std::uint64_t nanos) {
  const std::uint64_t before = nanos_;
  nanos_ += nanos;
  // Periodic events by the divider rate.
  const auto rs = static_cast<std::uint8_t>(regs_[kRegStatusA] & 0x0F);
  const int rate = kPeriodicRates[rs];
  if (rate > 0) {
    const std::uint64_t period = kNanosPerSecond / static_cast<std::uint64_t>(rate);
    periodic_nanos_ += nanos;
    while (periodic_nanos_ >= period) {
      periodic_nanos_ -= period;
      regs_[kRegStatusC] |= kStatusCPf;
    }
  }
  // Calendar updates on virtual second boundaries.
  const std::uint64_t seconds_before = before / kNanosPerSecond;
  const std::uint64_t seconds_after = nanos_ / kNanosPerSecond;
  for (std::uint64_t s = seconds_before + 1; s <= seconds_after; ++s) {
    if ((regs_[kRegStatusB] & kStatusBSet) == 0) {
      epoch_ += 1;
      SyncCalendar();
      regs_[kRegStatusC] |= kStatusCUf;
      if (AlarmMatches()) regs_[kRegStatusC] |= kStatusCAf;
    }
  }
  RaisePendingIrq();
}

void Rtc::InitCmos(std::uint64_t ram_bytes, bool floppy_present, bool fpu_present) {
  const std::uint64_t ram_kib = ram_bytes / 1024;
  regs_[0x10] = floppy_present ? 0x40 : 0x00;  // 1.44M drive or none
  regs_[0x14] = fpu_present ? 0x02 : 0x00;     // equipment byte
  const std::uint16_t base_kib = 640;
  regs_[0x15] = static_cast<std::uint8_t>(base_kib & 0xFF);
  regs_[0x16] = static_cast<std::uint8_t>((base_kib >> 8) & 0xFF);
  const std::uint16_t extended_kib =
      ram_kib > 1024 ? static_cast<std::uint16_t>(ram_kib - 1024) : 0;
  regs_[0x17] = static_cast<std::uint8_t>(extended_kib & 0xFF);
  regs_[0x18] = static_cast<std::uint8_t>((extended_kib >> 8) & 0xFF);
  // Memory above 16 MiB in 64 KiB blocks.
  const std::uint16_t blocks =
      ram_kib > 16384 ? static_cast<std::uint16_t>((ram_kib - 16384) / 64) : 0;
  regs_[0x34] = static_cast<std::uint8_t>(blocks & 0xFF);
  regs_[0x35] = static_cast<std::uint8_t>((blocks >> 8) & 0xFF);
  // Standard CMOS checksum over 0x10..0x2D at 0x2E/0x2F.
  std::uint16_t sum = 0;
  for (int i = 0x10; i <= 0x2D; ++i) sum = static_cast<std::uint16_t>(sum + regs_[i]);
  regs_[0x2E] = static_cast<std::uint8_t>(sum & 0xFF);
  regs_[0x2F] = static_cast<std::uint8_t>((sum >> 8) & 0xFF);
}

void Rtc::SyncCalendar() {
  const std::int64_t days = epoch_ / 86400;
  std::int64_t rem = epoch_ % 86400;
  if (rem < 0) rem += 86400;
  int year = 0;
  int month = 0;
  int day = 0;
  CivilFromDays(days, year, month, day);
  const int hour = static_cast<int>(rem / 3600);
  const int minute = static_cast<int>((rem % 3600) / 60);
  const int second = static_cast<int>(rem % 60);
  const int weekday = static_cast<int>((days + 4) % 7);  // 1970-01-01 was Thursday
  regs_[kRegSeconds] = Encode(static_cast<std::uint8_t>(second));
  regs_[kRegMinutes] = Encode(static_cast<std::uint8_t>(minute));
  if ((regs_[kRegStatusB] & kStatusB24Hour) == 0 && hour > 12) {
    regs_[kRegHours] =
        static_cast<std::uint8_t>(Encode(static_cast<std::uint8_t>(hour - 12)) | 0x80);
  } else if ((regs_[kRegStatusB] & kStatusB24Hour) == 0 && hour == 0) {
    regs_[kRegHours] = Encode(12);
  } else {
    regs_[kRegHours] = Encode(static_cast<std::uint8_t>(hour));
  }
  regs_[kRegDayOfWeek] = Encode(static_cast<std::uint8_t>(weekday == 0 ? 7 : weekday));
  regs_[kRegDay] = Encode(static_cast<std::uint8_t>(day));
  regs_[kRegMonth] = Encode(static_cast<std::uint8_t>(month));
  regs_[kRegYear] = Encode(static_cast<std::uint8_t>(year % 100));
}

bool Rtc::AlarmMatches() const {
  const auto matches = [this](std::uint8_t alarm_reg, std::uint8_t value) {
    const std::uint8_t alarm = regs_[alarm_reg];
    if (alarm >= 0xC0) return true;  // don't-care
    return alarm == value;
  };
  const std::int64_t days = epoch_ / 86400;
  std::int64_t rem = epoch_ % 86400;
  if (rem < 0) rem += 86400;
  const auto second = static_cast<std::uint8_t>(rem % 60);
  const auto minute = static_cast<std::uint8_t>((rem % 3600) / 60);
  const auto hour = static_cast<std::uint8_t>(rem / 3600);
  (void)days;
  return matches(0x01, Encode(second)) && matches(0x03, Encode(minute)) &&
         matches(0x05, Encode(hour));
}

void Rtc::RaisePendingIrq() {
  const std::uint8_t status_c = regs_[kRegStatusC];
  const std::uint8_t status_b = regs_[kRegStatusB];
  const bool enabled = ((status_c & kStatusCPf) != 0 && (status_b & kStatusBPie) != 0) ||
                       ((status_c & kStatusCAf) != 0 && (status_b & kStatusBAie) != 0) ||
                       ((status_c & kStatusCUf) != 0 && (status_b & kStatusBUie) != 0);
  if (enabled && !irq_line_) {
    irq_line_ = true;
    regs_[kRegStatusC] |= kStatusCIrqf;
    pic_.RaiseIrq(8);
  }
  if (!enabled) {
    irq_line_ = false;
  }
}

std::uint8_t Rtc::Encode(std::uint8_t value) const {
  if ((regs_[kRegStatusB] & kStatusBDm) != 0) return value;
  return static_cast<std::uint8_t>(((value / 10) << 4) | (value % 10));
}

int Rtc::year() const {
  const std::int64_t days = epoch_ / 86400;
  int year = 0;
  int month = 0;
  int day = 0;
  CivilFromDays(days, year, month, day);
  return year;
}

int Rtc::month() const {
  const std::int64_t days = epoch_ / 86400;
  int year = 0;
  int month = 0;
  int day = 0;
  CivilFromDays(days, year, month, day);
  return month;
}

int Rtc::day() const {
  const std::int64_t days = epoch_ / 86400;
  int year = 0;
  int month = 0;
  int day = 0;
  CivilFromDays(days, year, month, day);
  return day;
}

int Rtc::hours() const {
  std::int64_t rem = epoch_ % 86400;
  if (rem < 0) rem += 86400;
  return static_cast<int>(rem / 3600);
}

int Rtc::minutes() const {
  std::int64_t rem = epoch_ % 86400;
  if (rem < 0) rem += 86400;
  return static_cast<int>((rem % 3600) / 60);
}

int Rtc::seconds() const {
  std::int64_t rem = epoch_ % 86400;
  if (rem < 0) rem += 86400;
  return static_cast<int>(rem % 60);
}

std::uint8_t Rtc::ReadRegister(std::uint8_t index) {
  switch (index) {
    case kRegStatusA: {
      // UIP: last 8 quartz ticks before the calendar update; SET freezes
      // the calendar and keeps UIP cleared.
      const std::uint64_t into_second = nanos_ % kNanosPerSecond;
      const std::uint64_t ticks_into_second = into_second * kQuartzHz / kNanosPerSecond;
      const bool uip =
          (regs_[kRegStatusB] & kStatusBSet) == 0 && ticks_into_second >= kQuartzHz - 8;
      return static_cast<std::uint8_t>((regs_[kRegStatusA] & 0x7F) | (uip ? kStatusAUip : 0));
    }
    case kRegStatusC: {
      // Reading C returns the flags and clears them so the next event may
      // raise the interrupt again.
      const std::uint8_t value = regs_[kRegStatusC];
      regs_[kRegStatusC] = 0;
      irq_line_ = false;
      pic_.LowerIrq(8);
      return value;
    }
    case kRegStatusD:
      return regs_[kRegStatusD];
    case kRegSeconds:
    case kRegMinutes:
    case kRegHours:
    case kRegDayOfWeek:
    case kRegDay:
    case kRegMonth:
    case kRegYear:
    case kRegStatusB:
    default:
      // Time registers and the CMOS area read their storage directly.
      return regs_[index];
  }
}

void Rtc::WriteRegister(std::uint8_t index, std::uint8_t value) {
  switch (index) {
    case kRegStatusA:
      regs_[kRegStatusA] = static_cast<std::uint8_t>(value & 0x7F);
      return;
    case kRegStatusB:
      regs_[kRegStatusB] = value;
      if ((value & kStatusBSet) != 0) {
        // SET stops calendar updates and clears UIE.
        regs_[kRegStatusB] = static_cast<std::uint8_t>(value & ~kStatusBUie);
      }
      // Enabling an event with a saved flag raises the interrupt at once.
      RaisePendingIrq();
      return;
    case kRegStatusC:
    case kRegStatusD:
      return;  // C and D are read-only
    default:
      regs_[index] = value;
      return;
  }
}

std::uint32_t Rtc::ReadPort(std::uint16_t port, AccessWidth /*width*/) {
  if ((port & 1U) == 0) return nmi_mask_;
  return ReadRegister(index_);
}

void Rtc::WritePort(std::uint16_t port, std::uint32_t value, AccessWidth /*width*/) {
  const auto byte = static_cast<std::uint8_t>(value & 0xFF);
  if ((port & 1U) == 0) {
    // The NMI mask bit survives regardless of the selected register.
    nmi_mask_ = static_cast<std::uint8_t>(byte & 0x80);
    index_ = static_cast<std::uint8_t>(byte & 0x7F);
    return;
  }
  WriteRegister(index_, byte);
}

std::uint32_t Rtc::ReadMemory(std::uint32_t /*address*/, AccessWidth width) {
  return width == AccessWidth::k8 ? 0xFFU : (width == AccessWidth::k16 ? 0xFFFFU : 0xFFFFFFFFU);
}

void Rtc::WriteMemory(std::uint32_t /*address*/, std::uint32_t /*value*/, AccessWidth /*width*/) {}

}  // namespace ai386
