#include "machine/memory.hpp"

#include <stdexcept>

namespace ai386 {

Memory::Memory(std::uint64_t size_bytes) : bytes_(size_bytes, 0) {
  if (size_bytes == 0) {
    throw std::invalid_argument("RAM size must be positive");
  }
}

std::uint64_t Memory::size() const { return bytes_.size(); }

std::uint8_t Memory::Read8(std::uint32_t address) const {
  if (address >= bytes_.size()) return 0xFF;
  return bytes_[address];
}

void Memory::Write8(std::uint32_t address, std::uint8_t value) {
  if (address >= bytes_.size()) return;
  bytes_[address] = value;
}

std::uint16_t Memory::Read16(std::uint32_t address) const {
  const std::uint32_t lo = Read8(address);
  const std::uint32_t hi = Read8(address + 1);
  return static_cast<std::uint16_t>(lo | (hi << 8U));
}

void Memory::Write16(std::uint32_t address, std::uint16_t value) {
  Write8(address, static_cast<std::uint8_t>(value & 0xFFU));
  const std::uint32_t wide = value;
  Write8(address + 1, static_cast<std::uint8_t>((wide >> 8U) & 0xFFU));
}

std::uint32_t Memory::Read32(std::uint32_t address) const {
  const std::uint32_t lo = Read16(address);
  const std::uint32_t hi = Read16(address + 2);
  return lo | (hi << 16U);
}

void Memory::Write32(std::uint32_t address, std::uint32_t value) {
  Write16(address, static_cast<std::uint16_t>(value & 0xFFFFU));
  Write16(address + 2, static_cast<std::uint16_t>((value >> 16U) & 0xFFFFU));
}

}  // namespace ai386
