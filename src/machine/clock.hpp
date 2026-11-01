#pragma once

#include <cstdint>

namespace ai386 {

// Virtual time model of the machine: the guest clock frequency defines how
// many CPU cycles make a virtual nanosecond. Devices convert their own
// frequencies (PIT 1193182 Hz, RTC 32768 Hz quartz) from virtual time.
class VirtualClock {
 public:
  explicit VirtualClock(std::uint64_t clock_hz);

  [[nodiscard]] std::uint64_t clock_hz() const;
  // Total CPU cycles since machine start.
  [[nodiscard]] std::uint64_t cycles() const;
  // Virtual nanoseconds since machine start (scaled from cycles).
  [[nodiscard]] std::uint64_t nanos() const;

  // Advances time by the given number of CPU cycles.
  void Advance(std::uint64_t cycles);

 private:
  std::uint64_t clock_hz_;
  std::uint64_t cycles_ = 0;
  std::uint64_t nanos_ = 0;
  std::uint64_t nanos_remainder_ = 0;
};

}  // namespace ai386
