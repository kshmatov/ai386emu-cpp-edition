#include "machine/memory.hpp"

#include <gtest/gtest.h>

namespace ai386 {
namespace {

TEST(MemoryTest, ReadsAndWritesSingleByte) {
  Memory memory(1024);
  memory.Write8(0, 0xAB);
  EXPECT_EQ(memory.Read8(0), 0xAB);
  EXPECT_EQ(memory.Read8(1), 0x00);
}

TEST(MemoryTest, WideAccessIsLittleEndian) {
  Memory memory(1024);
  memory.Write32(4, 0x12345678);
  EXPECT_EQ(memory.Read8(4), 0x78);
  EXPECT_EQ(memory.Read8(5), 0x56);
  EXPECT_EQ(memory.Read8(6), 0x34);
  EXPECT_EQ(memory.Read8(7), 0x12);
  EXPECT_EQ(memory.Read16(4), 0x5678);
  EXPECT_EQ(memory.Read16(6), 0x1234);
  EXPECT_EQ(memory.Read32(4), 0x12345678);
}

TEST(MemoryTest, WideWriteAndReadRoundTrip) {
  Memory memory(1024);
  memory.Write16(0, 0xBEEF);
  EXPECT_EQ(memory.Read16(0), 0xBEEF);
  memory.Write32(2, 0xCAFEBABE);
  EXPECT_EQ(memory.Read32(2), 0xCAFEBABE);
}

TEST(MemoryTest, OutOfRangeAccessDoesNotCrash) {
  Memory memory(16);
  memory.Write8(15, 0x55);
  EXPECT_EQ(memory.Read8(15), 0x55);
  // Out-of-range reads answer 0xFF and writes are ignored.
  EXPECT_EQ(memory.Read8(16), 0xFF);
  EXPECT_EQ(memory.Read8(0xFFFFFFFF), 0xFF);
  memory.Write8(0xFFFFFFFF, 0x01);
  EXPECT_EQ(memory.Read8(0xFFFFFFFF), 0xFF);
}

}  // namespace
}  // namespace ai386
