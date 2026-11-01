#include "machine/clock.hpp"

#include <gtest/gtest.h>

namespace ai386 {
namespace {

TEST(ClockTest, CyclesAdvanceMonotonically) {
  VirtualClock clock(100000000);
  EXPECT_EQ(clock.cycles(), 0u);
  clock.Advance(100);
  EXPECT_EQ(clock.cycles(), 100u);
  clock.Advance(50);
  EXPECT_EQ(clock.cycles(), 150u);
}

TEST(ClockTest, NanosScaleToClockFrequency) {
  // 100 MHz: 100 cycles = 1 microsecond.
  VirtualClock clock(100000000);
  clock.Advance(100);
  EXPECT_EQ(clock.nanos(), 1000u);
}

TEST(ClockTest, FractionalNanosAccumulateWithoutLoss) {
  // 3 Hz: one cycle is 1/3 second; three cycles give exactly 1 second.
  VirtualClock clock(3);
  clock.Advance(1);
  clock.Advance(1);
  EXPECT_LT(clock.nanos(), 1000000000ULL);
  clock.Advance(1);
  EXPECT_EQ(clock.nanos(), 1000000000ULL);
}

TEST(ClockTest, ZeroClockRejected) { EXPECT_THROW(VirtualClock clock(0), std::invalid_argument); }

}  // namespace
}  // namespace ai386
