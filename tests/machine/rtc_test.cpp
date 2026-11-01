#include "machine/rtc.hpp"

#include <gtest/gtest.h>

namespace ai386 {
namespace {

// 2024-02-28 23:59:59 UTC in seconds since the Unix epoch.
constexpr std::int64_t kEpoch = 1709164799;

class RtcTest : public ::testing::Test {
 protected:
  RtcTest() : rtc_(pic_, kEpoch) {}

  void Select(std::uint8_t index) { rtc_.WritePort(0x70, index, AccessWidth::k8); }
  std::uint8_t Read(std::uint8_t index) {
    Select(index);
    return static_cast<std::uint8_t>(rtc_.ReadPort(0x71, AccessWidth::k8));
  }
  void Write(std::uint8_t index, std::uint8_t value) {
    Select(index);
    rtc_.WritePort(0x71, value, AccessWidth::k8);
  }

  Pic pic_;
  Rtc rtc_;
};

// Scenario: Календарь и SET
TEST_F(RtcTest, CalendarAdvancesAcrossLeapDay) {
  rtc_.Advance(1000000000ULL);  // one virtual second
  EXPECT_EQ(rtc_.year(), 2024);
  EXPECT_EQ(rtc_.month(), 2);
  EXPECT_EQ(rtc_.day(), 29);
  EXPECT_EQ(rtc_.hours(), 0);
  EXPECT_EQ(rtc_.minutes(), 0);
  EXPECT_EQ(rtc_.seconds(), 0);
}

// Scenario: Календарь и SET — SET=1 freezes the calendar and clears UIP
TEST_F(RtcTest, SetFreezesCalendarAndClearsUip) {
  Write(0x0B, 0x80);                   // SET=1
  rtc_.Advance(5000000000ULL);         // five virtual seconds
  EXPECT_EQ(rtc_.seconds(), 59);       // frozen at the start value
  EXPECT_EQ(Read(0x0A) & 0x80, 0x00);  // UIP clear while SET=1
}

// Scenario: Periodic IRQ и подтверждение
TEST_F(RtcTest, PeriodicIrqReachesGuestAndNeedsStatusCRead) {
  Write(0x0A, 0x26);                            // RS=6 -> 1024 Hz
  Write(0x0B, 0x40);                            // PIE=1
  pic_.WritePort(0xA0, 0x11, AccessWidth::k8);  // ICW1
  pic_.WritePort(0xA1, 0x70, AccessWidth::k8);  // vector base
  pic_.WritePort(0xA1, 0x02, AccessWidth::k8);
  pic_.WritePort(0xA1, 0x01, AccessWidth::k8);
  pic_.WritePort(0xA1, 0x00, AccessWidth::k8);  // unmask all slave lines

  rtc_.Advance(2000000ULL);  // 2 ms -> several 1024 Hz events
  auto vector = pic_.PendingVector();
  ASSERT_TRUE(vector.has_value());
  EXPECT_EQ(*vector, 0x70);            // IRQ8 -> slave vector base
  EXPECT_EQ(Read(0x0C) & 0x40, 0x40);  // PF set

  // Reading C cleared the flags: the next event must raise IRQ again.
  pic_.EndOfInterrupt();
  EXPECT_FALSE(pic_.PendingVector().has_value());
}

// Scenario: Отложенное разрешение
TEST_F(RtcTest, SavedFlagRaisesIrqWhenEnabled) {
  Write(0x0A, 0x26);         // RS=6 -> 1024 Hz divider runs
  Write(0x0B, 0x00);         // all events disabled
  rtc_.Advance(2000000ULL);  // periodic events fire but no IRQ
  EXPECT_FALSE(pic_.PendingVector().has_value());
  Write(0x0B, 0x40);  // enable PIE with a saved flag
  EXPECT_TRUE(pic_.PendingVector().has_value());
}

// Scenario: Alarm with don't-care values
TEST_F(RtcTest, AlarmMatchesWithDontCareBytes) {
  Write(0x01, 0xC0);                   // seconds don't-care
  Write(0x03, 0xC0);                   // minutes don't-care
  Write(0x05, 0xC0);                   // hours don't-care
  Write(0x0B, 0x20);                   // AIE=1
  rtc_.Advance(1000000000ULL);         // one virtual second triggers the alarm
  EXPECT_EQ(Read(0x0C) & 0x20, 0x20);  // AF set
}

// Scenario: CMOS соответствует памяти и floppy VM
TEST_F(RtcTest, CmosReportsMemoryAndFloppy) {
  rtc_.InitCmos(64ULL * 1024 * 1024, false, true);  // 64 MiB, no floppy
  EXPECT_EQ(Read(0x10), 0x00);                      // no floppy
  // Extended memory 64512 KiB at 0x17/0x18.
  const std::uint16_t extended = static_cast<std::uint16_t>(Read(0x17) | (Read(0x18) << 8));
  EXPECT_EQ(extended, 64512);
  // Above-16MiB memory 768 blocks of 64 KiB at 0x34/0x35.
  const std::uint16_t blocks = static_cast<std::uint16_t>(Read(0x34) | (Read(0x35) << 8));
  EXPECT_EQ(blocks, 768);
  // Checksum over 0x10..0x2D matches 0x2E/0x2F.
  std::uint16_t sum = 0;
  for (int i = 0x10; i <= 0x2D; ++i) sum = static_cast<std::uint16_t>(sum + Read(i));
  const std::uint16_t stored = static_cast<std::uint16_t>(Read(0x2E) | (Read(0x2F) << 8));
  EXPECT_EQ(sum, stored);
}

// Scenario: Floppy type 0x40 when a drive is present
TEST_F(RtcTest, CmosReportsFloppyType) {
  rtc_.InitCmos(16ULL * 1024 * 1024, true, true);
  EXPECT_EQ(Read(0x10), 0x40);
}

// Registers C and D are read-only.
TEST_F(RtcTest, StatusCAndDAreReadOnly) {
  Write(0x0C, 0x00);
  Write(0x0D, 0x00);
  EXPECT_EQ(Read(0x0D) & 0x80, 0x80);  // VRT still set
}

// The NMI mask bit of port 0x70 survives register index changes.
TEST_F(RtcTest, NmiMaskSurvivesIndexChange) {
  rtc_.WritePort(0x70, 0x85, AccessWidth::k8);  // NMI masked, index 5
  EXPECT_EQ(rtc_.ReadPort(0x70, AccessWidth::k8) & 0x80, 0x80);
  // Switching the register index keeps the mask when bit 7 is set.
  rtc_.WritePort(0x70, 0x8A, AccessWidth::k8);  // NMI masked, index 10
  EXPECT_EQ(rtc_.ReadPort(0x70, AccessWidth::k8) & 0x80, 0x80);
  rtc_.WritePort(0x70, 0x0A, AccessWidth::k8);  // unmasked, index 10
  EXPECT_EQ(rtc_.ReadPort(0x70, AccessWidth::k8) & 0x80, 0x00);
}

}  // namespace
}  // namespace ai386
