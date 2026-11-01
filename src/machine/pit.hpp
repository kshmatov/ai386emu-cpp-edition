#pragma once

#include <array>
#include <cstdint>

#include "machine/bus.hpp"
#include "machine/pic.hpp"

namespace ai386 {

// Intel 8253 programmable interval timer: three channels at 1193182 Hz on
// ports 0x40-0x43. Channel 0 raises IRQ0 through the PIC; channel 2 is
// gated by bit 0 of port 0x61 and reports its output on bit 5 of 0x61.
class Pit : public Device {
 public:
  static constexpr std::uint32_t kFrequencyHz = 1193182;

  explicit Pit(Pic& pic);

  // Advances timers by the given number of CPU cycles at clock_hz of the
  // machine; channel 0 raises IRQ0 on terminal count in modes 2 and 3.
  void Advance(std::uint64_t cycles, std::uint64_t clock_hz);

  std::uint32_t ReadMemory(std::uint32_t address, AccessWidth width) override;
  void WriteMemory(std::uint32_t address, std::uint32_t value, AccessWidth width) override;
  std::uint32_t ReadPort(std::uint16_t port, AccessWidth width) override;
  void WritePort(std::uint16_t port, std::uint32_t value, AccessWidth width) override;

 private:
  struct Channel {
    std::uint16_t reload = 0xFFFF;
    std::uint16_t counter = 0xFFFF;
    std::uint8_t mode = 2;
    std::uint8_t access = 3;  // 1 = lo, 2 = hi, 3 = lo/hi
    std::uint8_t latch = 0;   // 0 = unlatched, 1 = lo, 2 = hi
    std::uint16_t latched = 0;
    bool write_high = false;
    bool read_high = false;
    bool active = false;
  };

  void WriteControl(std::uint8_t value);
  void WriteChannel(int index, std::uint8_t value);
  std::uint8_t ReadChannel(int index);
  void TickChannel(int index, std::uint32_t ticks);

  Pic& pic_;
  std::array<Channel, 3> channels_{};
  std::uint64_t tick_remainder_ = 0;  // fractional PIT ticks from Advance
  bool gate2_ = true;
  bool out2_ = false;
};

}  // namespace ai386
