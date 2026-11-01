#pragma once

#include <cstdint>
#include <optional>

#include "machine/bus.hpp"

namespace ai386 {

// Intel 8259A programmable interrupt controller pair (master + slave on IRQ2).
// Devices raise IRQ lines; the CPU asks for the pending vector and the PIC
// keeps the line asserted until the CPU acknowledges with EOI.
class Pic : public Device {
 public:
  // IRQ lines 0-7 belong to the master, 8-15 to the slave.
  void RaiseIrq(std::uint8_t irq);
  void LowerIrq(std::uint8_t irq);

  // Vector of the highest-priority unmasked pending interrupt, if any.
  [[nodiscard]] std::optional<std::uint8_t> PendingVector() const;

  // Acknowledge: drops the in-service line of the highest priority so the
  // next interrupt can be delivered.
  void EndOfInterrupt();

  // The PIC is port-only; memory accesses behave as unmapped.
  std::uint32_t ReadMemory(std::uint32_t address, AccessWidth width) override;
  void WriteMemory(std::uint32_t address, std::uint32_t value, AccessWidth width) override;

  // Port access: master command/data at 0x20/0x21, slave at 0xA0/0xA1.
  std::uint32_t ReadPort(std::uint16_t port, AccessWidth width) override;
  void WritePort(std::uint16_t port, std::uint32_t value, AccessWidth width) override;

 private:
  struct Chip {
    std::uint8_t vector_base = 0x08;
    std::uint8_t mask = 0x00;        // OCW1: 1 = masked
    std::uint8_t request = 0x00;     // pending IRQ lines
    std::uint8_t in_service = 0x00;  // lines being served
    bool init4 = false;              // ICW4 expected during init
    std::uint8_t init_step = 0;      // 0 = idle, 1..3 = expecting ICW2..4
    bool auto_eoi = false;
  };

  [[nodiscard]] Chip& ChipOf(bool slave);
  [[nodiscard]] const Chip& ChipOf(bool slave) const;
  void WriteCommand(Chip& chip, std::uint8_t value);
  void WriteData(Chip& chip, std::uint8_t value);

  Chip master_{};
  Chip slave_{};
};

}  // namespace ai386
