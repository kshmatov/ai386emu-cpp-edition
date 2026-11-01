#pragma once

namespace ai386 {

// Processing unit driven by the machine run loop. The i486 implementation
// arrives with the CPU stage; the interface keeps machine-core independent
// of it.
class Cpu {
 public:
  virtual ~Cpu() = default;

  // Executes one instruction. Returns false when the CPU is halted (HLT)
  // and waits for an interrupt to be delivered before the next instruction.
  virtual bool Step() = 0;
};

}  // namespace ai386
