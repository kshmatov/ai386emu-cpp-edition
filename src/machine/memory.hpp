#pragma once

#include <cstdint>
#include <vector>

namespace ai386 {

// RAM of the virtual machine: a plain byte array covering [0, size).
class Memory {
 public:
  explicit Memory(std::uint64_t size_bytes);

  [[nodiscard]] std::uint64_t size() const;
  [[nodiscard]] std::uint8_t Read8(std::uint32_t address) const;
  void Write8(std::uint32_t address, std::uint8_t value);
  [[nodiscard]] std::uint16_t Read16(std::uint32_t address) const;
  void Write16(std::uint32_t address, std::uint16_t value);
  [[nodiscard]] std::uint32_t Read32(std::uint32_t address) const;
  void Write32(std::uint32_t address, std::uint32_t value);

 private:
  std::vector<std::uint8_t> bytes_;
};

}  // namespace ai386
