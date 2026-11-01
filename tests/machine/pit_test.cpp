#include "machine/pit.hpp"

#include <gtest/gtest.h>

namespace ai386 {
namespace {

class PitTest : public ::testing::Test {
 protected:
  PitTest() : pit_(pic_) {}

  // Writes a control word: channel, access lo/hi, mode.
  void Program(int channel, std::uint8_t access, std::uint8_t mode) {
    const std::uint8_t control =
        static_cast<std::uint8_t>((channel << 6) | (access << 4) | (mode << 1));
    pit_.WritePort(0x43, control, AccessWidth::k8);
  }

  Pic pic_;
  Pit pit_;
};

// Scenario: Периодический системный тик
TEST_F(PitTest, Channel0PeriodicTickRaisesIrq0) {
  Program(0, 3, 2);                             // lo/hi access, mode 2
  pit_.WritePort(0x40, 0xA9, AccessWidth::k8);  // reload 0x00A9 = 169
  pit_.WritePort(0x40, 0x00, AccessWidth::k8);
  // One PIT tick = 1193182 / clock_hz CPU cycles; at 1193182 Hz clock it is
  // exactly one cycle per tick.
  pit_.Advance(Pit::kFrequencyHz, Pit::kFrequencyHz);  // one full period
  EXPECT_TRUE(pic_.PendingVector().has_value());
}

// Scenario: Защёлка счётчика
TEST_F(PitTest, LatchKeepsBothBytesConsistent) {
  Program(0, 3, 2);
  pit_.WritePort(0x40, 0x34, AccessWidth::k8);
  pit_.WritePort(0x40, 0x12, AccessWidth::k8);  // reload 0x1234
  // Latch the counter while time advances between the two byte reads.
  pit_.WritePort(0x43, 0x00, AccessWidth::k8);  // latch channel 0
  const std::uint8_t lo = static_cast<std::uint8_t>(pit_.ReadPort(0x40, AccessWidth::k8));
  pit_.Advance(Pit::kFrequencyHz / 10, Pit::kFrequencyHz);  // advance between reads
  const std::uint8_t hi = static_cast<std::uint8_t>(pit_.ReadPort(0x40, AccessWidth::k8));
  const std::uint16_t value = static_cast<std::uint16_t>(lo | (hi << 8));
  EXPECT_EQ(value, 0x1234);
}

// Scenario: Канал 2 — gate через порт 0x61 и OUT на бите 5
TEST_F(PitTest, Channel2FollowsGateAndReportsOut) {
  Program(2, 3, 3);  // mode 3 square wave
  pit_.WritePort(0x42, 0x00, AccessWidth::k8);
  pit_.WritePort(0x42, 0x10, AccessWidth::k8);  // reload 0x1000
  pit_.WritePort(0x61, 0x01, AccessWidth::k8);  // gate on
  EXPECT_EQ(pit_.ReadPort(0x61, AccessWidth::k8) & 0x01, 0x01);
  // With the gate on the counter runs; without it the counter stops.
  pit_.WritePort(0x61, 0x00, AccessWidth::k8);
  EXPECT_EQ(pit_.ReadPort(0x61, AccessWidth::k8) & 0x01, 0x00);
  // OUT bit reflects the channel output level.
  EXPECT_EQ(pit_.ReadPort(0x61, AccessWidth::k8) & 0x20, 0x00);
}

TEST_F(PitTest, LoHiAccessReadsBothBytes) {
  Program(1, 3, 2);
  pit_.WritePort(0x41, 0xCD, AccessWidth::k8);
  pit_.WritePort(0x41, 0xAB, AccessWidth::k8);  // reload 0xABCD
  const std::uint8_t lo = static_cast<std::uint8_t>(pit_.ReadPort(0x41, AccessWidth::k8));
  const std::uint8_t hi = static_cast<std::uint8_t>(pit_.ReadPort(0x41, AccessWidth::k8));
  EXPECT_EQ(static_cast<std::uint16_t>(lo | (hi << 8)), 0xABCD);
}

TEST_F(PitTest, MemoryAccessDoesNotCrash) {
  EXPECT_EQ(pit_.ReadMemory(0, AccessWidth::k16), 0xFFFFu);
  pit_.WriteMemory(0, 0x12345678, AccessWidth::k32);
}

}  // namespace
}  // namespace ai386
