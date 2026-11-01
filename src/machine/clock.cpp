#include "machine/clock.hpp"

#include <stdexcept>

namespace ai386 {

namespace {
constexpr std::uint64_t kNanosPerSecond = 1000000000ULL;
}

VirtualClock::VirtualClock(std::uint64_t clock_hz) : clock_hz_(clock_hz) {
  if (clock_hz == 0) throw std::invalid_argument("clock_hz must be positive");
}

std::uint64_t VirtualClock::clock_hz() const { return clock_hz_; }

std::uint64_t VirtualClock::cycles() const { return cycles_; }

std::uint64_t VirtualClock::nanos() const { return nanos_; }

void VirtualClock::Advance(std::uint64_t cycles) {
  cycles_ += cycles;
  // Convert cycles to nanoseconds without losing the fractional part.
  const std::uint64_t total = nanos_remainder_ + cycles * kNanosPerSecond;
  nanos_ += total / clock_hz_;
  nanos_remainder_ = total % clock_hz_;
}

}  // namespace ai386
