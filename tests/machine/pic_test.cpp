#include "machine/pic.hpp"

#include <gtest/gtest.h>

namespace ai386 {
namespace {

TEST(PicTest, MasterAndSlaveVectorBases) {
  Pic pic;
  // ICW sequence: ICW1, vector base, ICW3, ICW4.
  auto init = [&pic](std::uint8_t base) {
    pic.WritePort(0x20, 0x11, AccessWidth::k8);  // ICW1
    pic.WritePort(0x21, base, AccessWidth::k8);  // ICW2
    pic.WritePort(0x21, 0x04, AccessWidth::k8);  // ICW3: slave on IRQ2
    pic.WritePort(0x21, 0x01, AccessWidth::k8);  // ICW4: 8086 mode
  };
  init(0x08);
  pic.WritePort(0xA0, 0x11, AccessWidth::k8);
  pic.WritePort(0xA1, 0x70, AccessWidth::k8);
  pic.WritePort(0xA1, 0x02, AccessWidth::k8);
  pic.WritePort(0xA1, 0x01, AccessWidth::k8);

  pic.RaiseIrq(3);
  auto vector = pic.PendingVector();
  ASSERT_TRUE(vector.has_value());
  EXPECT_EQ(*vector, 0x0B);  // master base 0x08 + IRQ3

  pic.EndOfInterrupt();
  pic.RaiseIrq(10);  // slave IRQ2 -> vector base + 2
  vector = pic.PendingVector();
  ASSERT_TRUE(vector.has_value());
  EXPECT_EQ(*vector, 0x72);
}

TEST(PicTest, MaskedIrqIsNotPending) {
  Pic pic;
  pic.WritePort(0x20, 0x11, AccessWidth::k8);
  pic.WritePort(0x21, 0x08, AccessWidth::k8);
  pic.WritePort(0x21, 0x04, AccessWidth::k8);
  pic.WritePort(0x21, 0x01, AccessWidth::k8);
  pic.WritePort(0x21, 0x08, AccessWidth::k8);  // mask IRQ3
  pic.RaiseIrq(3);
  EXPECT_FALSE(pic.PendingVector().has_value());
  // Reading the data port reports the mask.
  EXPECT_EQ(pic.ReadPort(0x21, AccessWidth::k8), 0x08u);
}

TEST(PicTest, EoiAllowsNextInterrupt) {
  Pic pic;
  pic.WritePort(0x20, 0x11, AccessWidth::k8);
  pic.WritePort(0x21, 0x08, AccessWidth::k8);
  pic.WritePort(0x21, 0x04, AccessWidth::k8);
  pic.WritePort(0x21, 0x01, AccessWidth::k8);

  pic.RaiseIrq(0);
  EXPECT_TRUE(pic.PendingVector().has_value());
  // Without EOI the request stays asserted; after EOI a new request delivers.
  pic.EndOfInterrupt();
  pic.RaiseIrq(0);
  EXPECT_TRUE(pic.PendingVector().has_value());
}

TEST(PicTest, HigherPriorityWins) {
  Pic pic;
  pic.WritePort(0x20, 0x11, AccessWidth::k8);
  pic.WritePort(0x21, 0x08, AccessWidth::k8);
  pic.WritePort(0x21, 0x04, AccessWidth::k8);
  pic.WritePort(0x21, 0x01, AccessWidth::k8);
  pic.RaiseIrq(7);
  pic.RaiseIrq(1);
  auto vector = pic.PendingVector();
  ASSERT_TRUE(vector.has_value());
  EXPECT_EQ(*vector, 0x09);  // IRQ1 has priority over IRQ7
}

TEST(PicTest, LowerIrqRemovesRequest) {
  Pic pic;
  pic.WritePort(0x20, 0x11, AccessWidth::k8);
  pic.WritePort(0x21, 0x08, AccessWidth::k8);
  pic.WritePort(0x21, 0x04, AccessWidth::k8);
  pic.WritePort(0x21, 0x01, AccessWidth::k8);
  pic.RaiseIrq(5);
  EXPECT_TRUE(pic.PendingVector().has_value());
  pic.LowerIrq(5);
  EXPECT_FALSE(pic.PendingVector().has_value());
}

TEST(PicTest, MemoryAccessDoesNotCrash) {
  Pic pic;
  EXPECT_EQ(pic.ReadMemory(0, AccessWidth::k8), 0xFFu);
  pic.WriteMemory(0, 0x12345678, AccessWidth::k32);
}

}  // namespace
}  // namespace ai386
