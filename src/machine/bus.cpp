#include "machine/bus.hpp"

#include <stdexcept>

namespace ai386 {
namespace {

std::uint8_t AccessBytes(AccessWidth width) {
  switch (width) {
    case AccessWidth::k8:
      return 1;
    case AccessWidth::k16:
      return 2;
    case AccessWidth::k32:
      return 4;
  }
  return 1;
}

std::uint32_t UnmappedValue(AccessWidth width) {
  switch (width) {
    case AccessWidth::k8:
      return 0xFF;
    case AccessWidth::k16:
      return 0xFFFF;
    case AccessWidth::k32:
      return 0xFFFFFFFF;
  }
  return 0xFF;
}

}  // namespace

Bus::Bus(Memory& ram) : ram_(ram) {}

void Bus::RegisterDevice(Device& device) {
  for (const Device* registered : devices_) {
    if (registered == &device) return;
  }
  devices_.push_back(&device);
}

const std::vector<Device*>& Bus::devices() const { return devices_; }

void Bus::AttachMemory(Device& device, std::uint32_t base, std::uint32_t size) {
  if (size == 0) throw std::invalid_argument("memory range size must be positive");
  const std::uint32_t end = base + size - 1;
  if (end < base) throw std::invalid_argument("memory range overflows 32-bit address space");
  for (const MemoryRange& range : memory_devices_) {
    const std::uint32_t range_end = range.base + range.size - 1;
    if (base <= range_end && range.base <= end) {
      throw std::invalid_argument("memory ranges of devices must not overlap");
    }
  }
  memory_devices_.push_back({.base = base, .size = size, .device = &device});
  RegisterDevice(device);
}

void Bus::AttachPort(Device& device, std::uint16_t base, std::uint16_t size) {
  if (size == 0) throw std::invalid_argument("port range size must be positive");
  const std::uint32_t end = static_cast<std::uint32_t>(base) + size - 1;
  for (const PortRange& range : port_devices_) {
    const std::uint32_t range_end = static_cast<std::uint32_t>(range.base) + range.size - 1;
    if (base <= range_end && range.base <= end) {
      throw std::invalid_argument("port ranges of devices must not overlap");
    }
  }
  port_devices_.push_back({.base = base, .size = size, .device = &device});
  RegisterDevice(device);
}

std::uint32_t Bus::ReadMemory(std::uint32_t address, AccessWidth width) {
  const std::uint8_t bytes = AccessBytes(width);
  for (const MemoryRange& range : memory_devices_) {
    if (address >= range.base && address + bytes <= range.base + range.size) {
      return range.device->ReadMemory(address, width);
    }
  }
  // RAM and unmapped addresses are composed from byte accesses, so a wide
  // access equals a little-endian byte sequence at adjacent addresses.
  std::uint32_t value = 0;
  for (std::uint8_t i = 0; i < bytes; ++i) {
    const std::uint8_t byte = ram_.Read8(address + i);
    value |= static_cast<std::uint32_t>(byte) << (8U * i);
  }
  return value;
}

void Bus::WriteMemory(std::uint32_t address, std::uint32_t value, AccessWidth width) {
  const std::uint8_t bytes = AccessBytes(width);
  for (const MemoryRange& range : memory_devices_) {
    if (address >= range.base && address + bytes <= range.base + range.size) {
      range.device->WriteMemory(address, value, width);
      return;
    }
  }
  for (std::uint8_t i = 0; i < bytes; ++i) {
    ram_.Write8(address + i, static_cast<std::uint8_t>((value >> (8U * i)) & 0xFFU));
  }
}

std::uint32_t Bus::ReadPort(std::uint16_t port, AccessWidth width) {
  const std::uint8_t bytes = AccessBytes(width);
  for (const PortRange& range : port_devices_) {
    const std::uint32_t range_end = static_cast<std::uint32_t>(range.base) + range.size - 1;
    if (port >= range.base && static_cast<std::uint32_t>(port) + bytes - 1 <= range_end) {
      return range.device->ReadPort(port, width);
    }
  }
  // Port reads without a device answer with 0xFF for the requested width.
  return UnmappedValue(width);
}

void Bus::WritePort(std::uint16_t port, std::uint32_t value, AccessWidth width) {
  const std::uint8_t bytes = AccessBytes(width);
  for (const PortRange& range : port_devices_) {
    const std::uint32_t range_end = static_cast<std::uint32_t>(range.base) + range.size - 1;
    if (port >= range.base && static_cast<std::uint32_t>(port) + bytes - 1 <= range_end) {
      range.device->WritePort(port, value, width);
    }
  }
}

}  // namespace ai386
