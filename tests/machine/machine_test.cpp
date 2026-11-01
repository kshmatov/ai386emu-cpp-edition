#include "machine/machine.hpp"

#include <gtest/gtest.h>

namespace ai386 {
namespace {

Config TestConfig() {
  Config config;
  config.hd_path = "test.img";
  config.ram_bytes = 1024 * 1024;
  return config;
}

// Scenario: Запуск машины — машина создаётся в состоянии сброса, готовая
// начать исполнение с адреса сброса процессора.
TEST(MachineTest, CreatedInResetStateNotRunning) {
  Machine machine(TestConfig());
  EXPECT_FALSE(machine.running());
}

// Scenario: Запуск машины
TEST(MachineTest, StartMovesToRunning) {
  Machine machine(TestConfig());
  machine.Start();
  EXPECT_TRUE(machine.running());
}

// Scenario: Остановка машины
TEST(MachineTest, StopReleasesResources) {
  Machine machine(TestConfig());
  machine.Start();
  machine.Stop();
  EXPECT_FALSE(machine.running());
}

// Scenario: Повторная остановка
TEST(MachineTest, StopIsIdempotent) {
  Machine machine(TestConfig());
  machine.Start();
  machine.Stop();
  EXPECT_NO_THROW(machine.Stop());
  EXPECT_FALSE(machine.running());
}

TEST(MachineTest, RamSizeFollowsConfig) {
  Config config = TestConfig();
  config.ram_bytes = 4 * 1024 * 1024;
  Machine machine(config);
  machine.bus().WriteMemory(0x3FFFFF, 0x42, AccessWidth::k8);
  EXPECT_EQ(machine.bus().ReadMemory(0x3FFFFF, AccessWidth::k8), 0x42u);
}

// Validation runs at machine creation too: the CD limit cannot be bypassed
// by direct Machine construction (spec: «Превышение лимита»).
TEST(MachineTest, InvalidConfigRejectedAtCreation) {
  Config config = TestConfig();
  config.ram_bytes = 0;
  EXPECT_THROW(Machine machine(config), ConfigError);
}

}  // namespace
}  // namespace ai386
