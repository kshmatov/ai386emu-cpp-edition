#pragma once

#include <cstdint>

#include "machine/bus.hpp"
#include "machine/pic.hpp"

namespace ai386 {

// Motorola MC146818-compatible real-time clock with 128 CMOS registers on
// ports 0x70/0x71. The calendar advances with virtual seconds; periodic,
// alarm and update-ended events raise IRQ8 through the PIC.
class Rtc : public Device {
 public:
  // start_epoch: seconds since the Unix epoch the calendar starts at.
  Rtc(Pic& pic, std::int64_t start_epoch);

  // Advances the calendar and raises timer events by virtual nanoseconds.
  void Advance(std::uint64_t nanos);

  // Fills the CMOS memory and equipment fields from the VM configuration.
  void InitCmos(std::uint64_t ram_bytes, bool floppy_present, bool fpu_present);

  // Current calendar fields (UTC).
  [[nodiscard]] int year() const;
  [[nodiscard]] int month() const;
  [[nodiscard]] int day() const;
  [[nodiscard]] int hours() const;
  [[nodiscard]] int minutes() const;
  [[nodiscard]] int seconds() const;

  std::uint32_t ReadMemory(std::uint32_t address, AccessWidth width) override;
  void WriteMemory(std::uint32_t address, std::uint32_t value, AccessWidth width) override;
  std::uint32_t ReadPort(std::uint16_t port, AccessWidth width) override;
  void WritePort(std::uint16_t port, std::uint32_t value, AccessWidth width) override;

 private:
  static constexpr std::uint8_t kRegSeconds = 0x00;
  static constexpr std::uint8_t kRegMinutes = 0x02;
  static constexpr std::uint8_t kRegHours = 0x04;
  static constexpr std::uint8_t kRegDayOfWeek = 0x06;
  static constexpr std::uint8_t kRegDay = 0x07;
  static constexpr std::uint8_t kRegMonth = 0x08;
  static constexpr std::uint8_t kRegYear = 0x09;
  static constexpr std::uint8_t kRegStatusA = 0x0A;
  static constexpr std::uint8_t kRegStatusB = 0x0B;
  static constexpr std::uint8_t kRegStatusC = 0x0C;
  static constexpr std::uint8_t kRegStatusD = 0x0D;

  std::uint8_t ReadRegister(std::uint8_t index);
  void WriteRegister(std::uint8_t index, std::uint8_t value);
  void SyncCalendar();
  void RaisePendingIrq();
  [[nodiscard]] bool AlarmMatches() const;
  [[nodiscard]] std::uint8_t Encode(std::uint8_t value) const;

  Pic& pic_;
  std::uint8_t regs_[128] = {};
  std::uint8_t index_ = 0;
  std::uint8_t nmi_mask_ = 0;  // bit 7 of port 0x70, kept independently
  std::int64_t epoch_ = 0;     // seconds since Unix epoch at virtual nanos 0
  std::uint64_t nanos_ = 0;    // virtual nanoseconds since start
  std::uint64_t periodic_nanos_ = 0;
  bool irq_line_ = false;
};

}  // namespace ai386
