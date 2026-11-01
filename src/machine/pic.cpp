#include "machine/pic.hpp"

namespace ai386 {
namespace {

constexpr std::uint8_t kIrqCascade = 2;  // master line wired to the slave

bool IsIcw1(std::uint8_t value) { return (value & 0x10) != 0; }
bool IsEoi(std::uint8_t value) { return (value & 0x20) != 0; }
bool IsSpecificEoi(std::uint8_t value) { return (value & 0x40) != 0; }

}  // namespace

Pic::Chip& Pic::ChipOf(bool slave) { return slave ? slave_ : master_; }

const Pic::Chip& Pic::ChipOf(bool slave) const { return slave ? slave_ : master_; }

void Pic::RaiseIrq(std::uint8_t irq) {
  Chip& chip = ChipOf(irq >= 8);
  chip.request |= static_cast<std::uint8_t>(1U << (irq & 7));
  // The slave feeds the master through the cascade line.
  if (irq >= 8) master_.request |= static_cast<std::uint8_t>(1U << kIrqCascade);
}

void Pic::LowerIrq(std::uint8_t irq) {
  Chip& chip = ChipOf(irq >= 8);
  chip.request &= static_cast<std::uint8_t>(~(1U << (irq & 7)));
  if (irq >= 8 && slave_.request == 0) {
    master_.request &= static_cast<std::uint8_t>(~(1U << kIrqCascade));
  }
}

std::optional<std::uint8_t> Pic::PendingVector() const {
  const auto slave_pending =
      static_cast<std::uint8_t>(slave_.request & static_cast<std::uint8_t>(~slave_.mask) &
                                static_cast<std::uint8_t>(~slave_.in_service));
  const auto master_pending =
      static_cast<std::uint8_t>(master_.request & static_cast<std::uint8_t>(~master_.mask) &
                                static_cast<std::uint8_t>(~master_.in_service));
  const auto cascade_bit = static_cast<std::uint8_t>(1U << kIrqCascade);
  // Fixed priority: IRQ0-1, then the slave through the cascade line, then
  // the remaining master lines.
  for (std::uint8_t i = 0; i < 8; ++i) {
    if (i == kIrqCascade) continue;
    if ((master_pending & (1U << i)) != 0) {
      return static_cast<std::uint8_t>(master_.vector_base + i);
    }
  }
  if (slave_pending != 0 && (master_pending & cascade_bit) != 0) {
    for (std::uint8_t i = 0; i < 8; ++i) {
      if ((slave_pending & (1U << i)) != 0) {
        return static_cast<std::uint8_t>(slave_.vector_base + i);
      }
    }
  }
  return std::nullopt;
}

void Pic::EndOfInterrupt() {
  // Simplified acknowledge: drop the highest-priority requested line so the
  // next interrupt can be delivered. CPU-side INTA and the in-service
  // register arrive with the CPU stage.
  const auto cascade_bit = static_cast<std::uint8_t>(1U << kIrqCascade);
  const std::uint8_t slave_pending = slave_.request & static_cast<std::uint8_t>(~slave_.mask);
  const std::uint8_t master_pending = master_.request & static_cast<std::uint8_t>(~master_.mask);
  for (std::uint8_t i = 0; i < 8; ++i) {
    if (i == kIrqCascade) continue;
    if ((master_pending & (1U << i)) != 0) {
      master_.request &= static_cast<std::uint8_t>(~(1U << i));
      return;
    }
  }
  if (slave_pending != 0 && (master_pending & cascade_bit) != 0) {
    for (std::uint8_t i = 0; i < 8; ++i) {
      if ((slave_pending & (1U << i)) != 0) {
        slave_.request &= static_cast<std::uint8_t>(~(1U << i));
        if (slave_.request == 0) master_.request &= static_cast<std::uint8_t>(~cascade_bit);
        return;
      }
    }
  }
}

std::uint32_t Pic::ReadMemory(std::uint32_t /*address*/, AccessWidth width) {
  return width == AccessWidth::k8 ? 0xFFU : (width == AccessWidth::k16 ? 0xFFFFU : 0xFFFFFFFFU);
}

void Pic::WriteMemory(std::uint32_t /*address*/, std::uint32_t /*value*/, AccessWidth /*width*/) {}

std::uint32_t Pic::ReadPort(std::uint16_t port, AccessWidth /*width*/) {
  const bool slave = port >= 0xA0;
  Chip& chip = ChipOf(slave);
  if ((port & 1U) == 1) return chip.mask;
  // Command port: report the request register (IRR) — the default read.
  return chip.request;
}

void Pic::WritePort(std::uint16_t port, std::uint32_t value, AccessWidth /*width*/) {
  const bool slave = port >= 0xA0;
  Chip& chip = ChipOf(slave);
  const auto byte = static_cast<std::uint8_t>(value & 0xFF);
  if ((port & 1U) == 0) {
    WriteCommand(chip, byte);
  } else {
    WriteData(chip, byte);
  }
}

void Pic::WriteCommand(Chip& chip, std::uint8_t value) {
  if (IsIcw1(value)) {
    chip.init_step = 1;
    chip.init4 = (value & 0x01) != 0;
    chip.mask = 0;
    chip.in_service = 0;
    chip.auto_eoi = false;
    return;
  }
  if (IsEoi(value)) {
    if (IsSpecificEoi(value)) {
      const auto level = static_cast<std::uint8_t>(value & 0x07);
      chip.in_service &= static_cast<std::uint8_t>(~(1U << level));
    } else {
      for (std::uint8_t i = 0; i < 8; ++i) {
        const auto bit = static_cast<std::uint8_t>(1U << i);
        if ((chip.in_service & bit) != 0) {
          chip.in_service &= static_cast<std::uint8_t>(~bit);
          break;
        }
      }
    }
    return;
  }
  // OCW3: read selection is not modelled; IRR is always the command read.
}

void Pic::WriteData(Chip& chip, std::uint8_t value) {
  if (chip.init_step == 1) {
    chip.vector_base = static_cast<std::uint8_t>(value & 0xF8);
    chip.init_step = 2;
    return;
  }
  if (chip.init_step == 2) {
    // ICW3: cascade identity; no state needed beyond the wiring model.
    chip.init_step = chip.init4 ? 3 : 0;
    return;
  }
  if (chip.init_step == 3) {
    chip.auto_eoi = (value & 0x02) != 0;
    chip.init_step = 0;
    return;
  }
  chip.mask = value;
}

}  // namespace ai386
