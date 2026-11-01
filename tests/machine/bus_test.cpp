#include "machine/bus.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

namespace ai386 {
namespace {

// Fake device recording the accesses routed to it.
class RecordingDevice : public Device {
 public:
  std::uint32_t ReadMemory(std::uint32_t offset, AccessWidth width) override {
    memory_reads.push_back({offset, 0, width});
    return 0x5A5A5A5A;
  }
  void WriteMemory(std::uint32_t offset, std::uint32_t value, AccessWidth width) override {
    memory_writes.push_back({offset, value, width});
  }
  std::uint32_t ReadPort(std::uint16_t offset, AccessWidth width) override {
    port_reads.push_back({offset, 0, width});
    return 0x12345678;
  }
  void WritePort(std::uint16_t offset, std::uint32_t value, AccessWidth width) override {
    port_writes.push_back({offset, value, width});
  }

  struct Access {
    std::uint32_t offset;
    std::uint32_t value;
    AccessWidth width;
  };
  std::vector<Access> memory_reads;
  std::vector<Access> memory_writes;
  std::vector<Access> port_reads;
  std::vector<Access> port_writes;
};

class BusTest : public ::testing::Test {
 protected:
  BusTest() : ram_(4096), bus_(ram_) {}

  Memory ram_;
  Bus bus_;
};

// Scenario: Чтение и запись в RAM
TEST_F(BusTest, RamReadWrite) {
  bus_.WriteMemory(0x100, 0xAB, AccessWidth::k8);
  EXPECT_EQ(bus_.ReadMemory(0x100, AccessWidth::k8), 0xABu);
  bus_.WriteMemory(0x200, 0x12345678, AccessWidth::k32);
  EXPECT_EQ(bus_.ReadMemory(0x200, AccessWidth::k32), 0x12345678u);
  EXPECT_EQ(bus_.ReadMemory(0x200, AccessWidth::k16), 0x5678u);
}

// Scenario: Обращение к памяти устройства
TEST_F(BusTest, DeviceMemoryRangeTakesPrecedence) {
  RecordingDevice device;
  bus_.AttachMemory(device, 0x100, 16);
  bus_.WriteMemory(0x104, 0x42, AccessWidth::k8);
  EXPECT_EQ(bus_.ReadMemory(0x104, AccessWidth::k8), 0x5A5A5A5Au);
  ASSERT_EQ(device.memory_reads.size(), 1u);
  EXPECT_EQ(device.memory_reads[0].offset, 4u);
  EXPECT_EQ(device.memory_writes[0].offset, 4u);
  EXPECT_EQ(device.memory_writes[0].value, 0x42u);
  // RAM behind the device range is not touched.
  EXPECT_EQ(ram_.Read8(0x104), 0x00);
}

// Scenario: Недоступный адрес
TEST_F(BusTest, UnmappedMemoryIsPredictable) {
  // Reads return 0xFF-filled values; writes are ignored; nothing crashes.
  EXPECT_EQ(bus_.ReadMemory(0x10000, AccessWidth::k8), 0xFFu);
  EXPECT_EQ(bus_.ReadMemory(0x10000, AccessWidth::k16), 0xFFFFu);
  EXPECT_EQ(bus_.ReadMemory(0x10000, AccessWidth::k32), 0xFFFFFFFFu);
  bus_.WriteMemory(0x10000, 0x12345678, AccessWidth::k32);
  EXPECT_EQ(bus_.ReadMemory(0x10000, AccessWidth::k32), 0xFFFFFFFFu);
}

// Scenario: Подключение устройства с диапазоном портов
TEST_F(BusTest, PortAccessRoutedToDevice) {
  RecordingDevice device;
  bus_.AttachPort(device, 0x220, 16);
  bus_.WritePort(0x222, 0x77, AccessWidth::k8);
  EXPECT_EQ(bus_.ReadPort(0x222, AccessWidth::k8), 0x12345678u);
  ASSERT_EQ(device.port_writes.size(), 1u);
  EXPECT_EQ(device.port_writes[0].offset, 2u);
  EXPECT_EQ(device.port_writes[0].value, 0x77u);
  ASSERT_EQ(device.port_reads.size(), 1u);
  EXPECT_EQ(device.port_reads[0].offset, 2u);
}

// Scenario: Чтение порта без устройства
TEST_F(BusTest, UnmappedPortReadReturns0xFF) {
  EXPECT_EQ(bus_.ReadPort(0x300, AccessWidth::k8), 0xFFu);
  EXPECT_EQ(bus_.ReadPort(0x300, AccessWidth::k16), 0xFFFFu);
  EXPECT_EQ(bus_.ReadPort(0x300, AccessWidth::k32), 0xFFFFFFFFu);
  // Unmapped port writes are ignored without a crash.
  bus_.WritePort(0x300, 0x12345678, AccessWidth::k32);
}

// Scenario: Широкая запись и чтение в RAM
TEST_F(BusTest, WideRamAccessIsLittleEndian) {
  bus_.WriteMemory(0, 0x11223344, AccessWidth::k32);
  EXPECT_EQ(ram_.Read8(0), 0x44);
  EXPECT_EQ(ram_.Read8(1), 0x33);
  EXPECT_EQ(ram_.Read8(2), 0x22);
  EXPECT_EQ(ram_.Read8(3), 0x11);
  EXPECT_EQ(bus_.ReadMemory(0, AccessWidth::k32), 0x11223344u);
  EXPECT_EQ(bus_.ReadMemory(2, AccessWidth::k16), 0x1122u);
}

// Scenario: Широкий доступ к порту с устройством
TEST_F(BusTest, WidePortAccessPassesWidthToDevice) {
  RecordingDevice device;
  bus_.AttachPort(device, 0x1F0, 8);
  bus_.WritePort(0x1F0, 0xDEADBEEF, AccessWidth::k32);
  EXPECT_EQ(bus_.ReadPort(0x1F0, AccessWidth::k32), 0x12345678u);
  ASSERT_EQ(device.port_writes.size(), 1u);
  EXPECT_EQ(device.port_writes[0].width, AccessWidth::k32);
  EXPECT_EQ(device.port_writes[0].value, 0xDEADBEEFu);
  ASSERT_EQ(device.port_reads.size(), 1u);
  EXPECT_EQ(device.port_reads[0].width, AccessWidth::k32);
}

// Scenario: Широкий доступ к диапазону памяти устройства
TEST_F(BusTest, WideMemoryAccessPassesWidthToDevice) {
  RecordingDevice device;
  bus_.AttachMemory(device, 0x8000, 32);
  bus_.ReadMemory(0x8008, AccessWidth::k16);
  bus_.WriteMemory(0x8008, 0xAAAA, AccessWidth::k16);
  ASSERT_EQ(device.memory_reads.size(), 1u);
  EXPECT_EQ(device.memory_reads[0].width, AccessWidth::k16);
  ASSERT_EQ(device.memory_writes.size(), 1u);
  EXPECT_EQ(device.memory_writes[0].width, AccessWidth::k16);
  EXPECT_EQ(device.memory_writes[0].value, 0xAAAAu);
}

// Scenario: Широкий доступ вне памяти и устройств
TEST_F(BusTest, WideUnmappedAccessDoesNotCrash) {
  EXPECT_EQ(bus_.ReadMemory(0xF0000000, AccessWidth::k32), 0xFFFFFFFFu);
  EXPECT_EQ(bus_.ReadPort(0x9000, AccessWidth::k16), 0xFFFFu);
  bus_.WriteMemory(0xF0000000, 0x01020304, AccessWidth::k32);
  bus_.WritePort(0x9000, 0x01020304, AccessWidth::k32);
}

TEST_F(BusTest, OverlappingMemoryRangesAreRejected) {
  RecordingDevice first;
  RecordingDevice second;
  bus_.AttachMemory(first, 0x100, 16);
  EXPECT_THROW(bus_.AttachMemory(second, 0x108, 16), std::invalid_argument);
}

TEST_F(BusTest, OverlappingPortRangesAreRejected) {
  RecordingDevice first;
  RecordingDevice second;
  bus_.AttachPort(first, 0x220, 16);
  EXPECT_THROW(bus_.AttachPort(second, 0x228, 16), std::invalid_argument);
}

TEST_F(BusTest, ZeroSizeRangeIsRejected) {
  RecordingDevice device;
  EXPECT_THROW(bus_.AttachMemory(device, 0, 0), std::invalid_argument);
  EXPECT_THROW(bus_.AttachPort(device, 0, 0), std::invalid_argument);
}

}  // namespace
}  // namespace ai386
