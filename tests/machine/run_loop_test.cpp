#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <thread>
#include <utility>

#include "machine/machine.hpp"

namespace ai386 {
namespace {

Config TestConfig() {
  Config config;
  config.hd_path = "test.img";
  config.ram_bytes = 1024 * 1024;
  return config;
}

// CPU whose steps are defined by a callback: the callback receives the step
// index and answers whether the instruction was executed (false = halted).
class CallbackCpu : public Cpu {
 public:
  explicit CallbackCpu(std::function<bool(std::size_t)> step) : step_(std::move(step)) {}

  bool Step() override { return step_(step_count_++); }

  std::size_t step_count() const { return step_count_; }

 private:
  std::function<bool(std::size_t)> step_;
  std::size_t step_count_ = 0;
};

// Waits until the predicate holds or the timeout expires.
bool WaitUntil(const std::function<bool()>& predicate, std::chrono::milliseconds timeout) {
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  while (!predicate()) {
    if (std::chrono::steady_clock::now() > deadline) return false;
    std::this_thread::yield();
  }
  return true;
}

// Scenario: Результат гостевого кода — цикл исполняет инструкции до запроса
// остановки и завершается штатно.
TEST(MachineRunLoopTest, RunExecutesUntilStopRequested) {
  Machine machine(TestConfig());
  CallbackCpu cpu([&machine](std::size_t index) {
    if (index == 2) machine.RequestStop();
    return true;
  });
  EXPECT_NO_THROW(machine.Run(cpu));
  EXPECT_EQ(cpu.step_count(), 3u);
  EXPECT_FALSE(machine.running());
}

// Scenario: Остановка активного гостя — остановка из другого потока
// завершает исполнение и освобождает ресурсы.
TEST(MachineRunLoopTest, StopFromAnotherThreadFinishesRun) {
  Machine machine(TestConfig());
  CallbackCpu cpu([](std::size_t) { return true; });
  std::thread stopper([&machine, &cpu] {
    ASSERT_TRUE(WaitUntil([&cpu] { return cpu.step_count() > 0; }, std::chrono::seconds(5)));
    machine.RequestStop();
  });
  EXPECT_NO_THROW(machine.Run(cpu));
  stopper.join();
  EXPECT_FALSE(machine.running());
}

// Scenario: Ожидание после HLT — машина остаётся запущенной и доступной
// для остановки, не исполняет следующие инструкции до пробуждения CPU.
TEST(MachineRunLoopTest, HaltedCpuWaitsForWake) {
  Machine machine(TestConfig());
  std::atomic<bool> woken{false};
  // Step 0 halts; further steps only run after Wake().
  CallbackCpu cpu([&woken](std::size_t index) {
    if (index == 0) return false;
    EXPECT_TRUE(woken.load());
    return index < 2;
  });
  std::thread waker([&machine, &woken, &cpu] {
    ASSERT_TRUE(WaitUntil([&cpu] { return cpu.step_count() >= 1; }, std::chrono::seconds(5)));
    // While the CPU is halted the run loop must not execute instructions.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    woken.store(true);
    machine.Wake();
    ASSERT_TRUE(WaitUntil([&cpu] { return cpu.step_count() >= 2; }, std::chrono::seconds(5)));
    machine.RequestStop();
  });
  EXPECT_NO_THROW(machine.Run(cpu));
  waker.join();
  // Halt, wake, execute one instruction, halt again, stop: at most 3 steps.
  EXPECT_LE(cpu.step_count(), 3u);
}

// Scenario: Повторный запуск цикла — второй цикл отклоняется.
TEST(MachineRunLoopTest, SecondRunLoopRejected) {
  Machine machine(TestConfig());
  CallbackCpu first([](std::size_t) { return true; });
  std::thread runner([&machine, &first] { machine.Run(first); });
  ASSERT_TRUE(WaitUntil([&machine] { return machine.running(); }, std::chrono::seconds(5)));
  CallbackCpu second([](std::size_t) { return true; });
  EXPECT_THROW(machine.Run(second), std::logic_error);
  machine.RequestStop();
  runner.join();
}

// Scenario: Повторный запуск цикла — после завершения новый цикл разрешён.
TEST(MachineRunLoopTest, RunAgainAfterFinishIsAllowed) {
  Machine machine(TestConfig());
  CallbackCpu first([&machine](std::size_t index) {
    if (index == 1) machine.RequestStop();
    return true;
  });
  EXPECT_NO_THROW(machine.Run(first));
  CallbackCpu second([&machine](std::size_t index) {
    if (index == 1) machine.RequestStop();
    return true;
  });
  EXPECT_NO_THROW(machine.Run(second));
  EXPECT_EQ(second.step_count(), 2u);
}

// Scenario: Остановка активного гостя — остановка после HLT завершает цикл.
TEST(MachineRunLoopTest, StopWhileHaltedFinishesRun) {
  Machine machine(TestConfig());
  CallbackCpu cpu([](std::size_t) { return false; });  // always halted
  std::thread stopper([&machine, &cpu] {
    ASSERT_TRUE(WaitUntil([&cpu] { return cpu.step_count() >= 1; }, std::chrono::seconds(5)));
    machine.RequestStop();
  });
  EXPECT_NO_THROW(machine.Run(cpu));
  stopper.join();
  EXPECT_FALSE(machine.running());
}

}  // namespace
}  // namespace ai386
