#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "machine/memory.hpp"

namespace ai386 {

// Access width of a bus transaction in bits.
enum class AccessWidth : std::uint8_t { k8 = 8, k16 = 16, k32 = 32 };

// A peripheral attached to the bus: a memory range and/or a port range. The
// device receives the absolute address or port of the access; wide accesses
// are passed through as a whole so devices like the IDE data register keep
// their FIFO semantics.
class Device {
 public:
  virtual ~Device() = default;

  // Memory-mapped register access inside a registered range.
  virtual std::uint32_t ReadMemory(std::uint32_t address, AccessWidth width) = 0;
  virtual void WriteMemory(std::uint32_t address, std::uint32_t value, AccessWidth width) = 0;

  // Port access inside a registered port range.
  virtual std::uint32_t ReadPort(std::uint16_t port, AccessWidth width) = 0;
  virtual void WritePort(std::uint16_t port, std::uint32_t value, AccessWidth width) = 0;

  // Lifecycle notifications from the machine.
  virtual void Start() {}
  virtual void Stop() {}
};

// System bus: routes memory and port accesses between RAM and registered
// devices. Unmapped reads return 0xFF filled for the access width; unmapped
// writes are ignored, so a guest can never crash the emulator from the bus.
class Bus {
 public:
  explicit Bus(Memory& ram);

  // Registers a device over a RAM address range or a port range. Ranges of
  // different devices may not overlap.
  void AttachMemory(Device& device, std::uint32_t base, std::uint32_t size);
  void AttachPort(Device& device, std::uint16_t base, std::uint16_t size);

  std::uint32_t ReadMemory(std::uint32_t address, AccessWidth width);
  void WriteMemory(std::uint32_t address, std::uint32_t value, AccessWidth width);
  std::uint32_t ReadPort(std::uint16_t port, AccessWidth width);
  void WritePort(std::uint16_t port, std::uint32_t value, AccessWidth width);

  // Registered devices, for lifecycle notifications from the machine.
  [[nodiscard]] const std::vector<Device*>& devices() const;

 private:
  struct MemoryRange {
    std::uint32_t base;
    std::uint32_t size;
    Device* device;
  };
  struct PortRange {
    std::uint16_t base;
    std::uint16_t size;
    Device* device;
  };

  void RegisterDevice(Device& device);

  Memory& ram_;
  std::vector<MemoryRange> memory_devices_;
  std::vector<PortRange> port_devices_;
  std::vector<Device*> devices_;
};

}  // namespace ai386
