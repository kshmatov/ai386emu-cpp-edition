#include "machine/pit.hpp"

namespace ai386 {
namespace {

constexpr std::uint8_t kAccessLo = 1;
constexpr std::uint8_t kAccessHi = 2;
constexpr std::uint8_t kAccessLoHi = 3;
constexpr std::uint8_t kLatchCommand = 0;

std::uint16_t EffectiveReload(const std::uint16_t reload) {
  // A programmed value of 0 means 65536 in real hardware.
  return reload == 0 ? 0xFFFF : reload;
}

}  // namespace

Pit::Pit(Pic& pic) : pic_(pic) {
  for (Channel& channel : channels_) {
    channel.counter = channel.reload;
  }
}

void Pit::Advance(std::uint64_t cycles, std::uint64_t clock_hz) {
  // Convert CPU cycles to PIT ticks at 1193182 Hz, keeping the fraction.
  const std::uint64_t total = tick_remainder_ + cycles * kFrequencyHz;
  const auto ticks = static_cast<std::uint32_t>(total / clock_hz);
  tick_remainder_ = total % clock_hz;
  if (ticks == 0) return;
  TickChannel(0, ticks);
  TickChannel(1, ticks);
  if (gate2_) TickChannel(2, ticks);
}

void Pit::TickChannel(int index, std::uint32_t ticks) {
  Channel& channel = channels_[index];
  if (!channel.active) return;
  const std::uint16_t reload = EffectiveReload(channel.reload);
  std::uint32_t remaining = ticks;
  while (remaining > 0) {
    if (channel.counter > remaining) {
      channel.counter = static_cast<std::uint16_t>(channel.counter - remaining);
      remaining = 0;
    } else {
      remaining -= channel.counter;
      channel.counter = reload;
      if (index == 2) out2_ = !out2_;
      if (index == 0) pic_.RaiseIrq(0);
      if (channel.mode == 0) {
        // One-shot: stop counting after the terminal count.
        channel.active = false;
        remaining = 0;
      }
    }
  }
}

void Pit::WriteControl(std::uint8_t value) {
  const int index = static_cast<int>((value >> 6) & 0x03);
  if (index > 2) return;  // read-back command is not modelled
  Channel& channel = channels_[index];
  const auto access = static_cast<std::uint8_t>((value >> 4) & 0x03);
  if (access == kLatchCommand) {
    channel.latched = channel.counter;
    channel.latch = channel.access;  // latch follows the programmed access
    return;
  }
  channel.access = access;
  channel.mode = static_cast<std::uint8_t>((value >> 1) & 0x07);
  channel.write_high = false;
  channel.read_high = false;
  channel.latch = 0;
  channel.active = true;
  if (index == 2) out2_ = false;
}

void Pit::WriteChannel(int index, std::uint8_t value) {
  Channel& channel = channels_[index];
  if (channel.access == kAccessLo) {
    channel.reload = value;
  } else if (channel.access == kAccessHi) {
    channel.reload = static_cast<std::uint16_t>(value << 8);
  } else {
    if (!channel.write_high) {
      channel.reload = static_cast<std::uint16_t>((channel.reload & 0xFF00) | value);
      channel.write_high = true;
    } else {
      channel.reload = static_cast<std::uint16_t>((channel.reload & 0x00FF) |
                                                  (static_cast<std::uint16_t>(value) << 8));
      channel.write_high = false;
    }
  }
  // The counter starts from the newly written value once both bytes arrive.
  if (channel.access == kAccessLo || channel.access == kAccessHi || !channel.write_high) {
    channel.counter = EffectiveReload(channel.reload);
    channel.active = true;
    if (index == 2) out2_ = false;
  }
}

std::uint8_t Pit::ReadChannel(int index) {
  Channel& channel = channels_[index];
  const bool latched = channel.latch != 0;
  const std::uint16_t value = latched ? channel.latched : channel.counter;
  const std::uint8_t access = latched ? channel.latch : channel.access;
  if (access == kAccessLo) {
    if (latched) channel.latch = 0;
    return static_cast<std::uint8_t>(value & 0xFF);
  }
  if (access == kAccessHi) {
    if (latched) channel.latch = 0;
    return static_cast<std::uint8_t>((value >> 8) & 0xFF);
  }
  // lo/hi access: two reads deliver both bytes of one counter value.
  if (!channel.read_high) {
    channel.read_high = true;
    return static_cast<std::uint8_t>(value & 0xFF);
  }
  channel.read_high = false;
  if (latched) channel.latch = 0;
  return static_cast<std::uint8_t>((value >> 8) & 0xFF);
}

std::uint32_t Pit::ReadPort(std::uint16_t port, AccessWidth /*width*/) {
  if (port == 0x61) {
    std::uint8_t value = 0;
    if (gate2_) value |= 0x01;
    if (out2_) value |= 0x20;
    return value;
  }
  return ReadChannel(static_cast<int>(port - 0x40));
}

void Pit::WritePort(std::uint16_t port, std::uint32_t value, AccessWidth /*width*/) {
  const auto byte = static_cast<std::uint8_t>(value & 0xFF);
  if (port == 0x61) {
    gate2_ = (byte & 0x01) != 0;
    return;
  }
  if (port == 0x43) {
    WriteControl(byte);
    return;
  }
  WriteChannel(static_cast<int>(port - 0x40), byte);
}

std::uint32_t Pit::ReadMemory(std::uint32_t /*address*/, AccessWidth width) {
  return width == AccessWidth::k8 ? 0xFFU : (width == AccessWidth::k16 ? 0xFFFFU : 0xFFFFFFFFU);
}

void Pit::WriteMemory(std::uint32_t /*address*/, std::uint32_t /*value*/, AccessWidth /*width*/) {}

}  // namespace ai386
