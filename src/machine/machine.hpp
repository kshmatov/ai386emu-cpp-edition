#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "config/config.hpp"
#include "machine/bus.hpp"
#include "machine/cpu.hpp"
#include "machine/memory.hpp"

namespace ai386 {

// Virtual machine: owns RAM, the system bus and the registered devices.
class Machine {
 public:
  explicit Machine(const Config& config);
  ~Machine();

  Machine(const Machine&) = delete;
  Machine& operator=(const Machine&) = delete;

  // Moves the machine and all attached devices to the running state.
  void Start();
  // Stops devices and releases host resources (open image files). Calling
  // Stop on an already stopped machine is a no-op.
  void Stop();

  // Executes the guest continuously until RequestStop() is called and then
  // stops the machine, so media are released before the caller exits. Only
  // one run loop per machine is allowed: a second request is rejected.
  void Run(Cpu& cpu);

  // Requests the run loop to finish. Idempotent and safe to call from a
  // POSIX signal handler and from other threads.
  void RequestStop();

  // Wakes a halted (HLT) CPU, e.g. when a device asserts an interrupt.
  void Wake();

  [[nodiscard]] bool running() const;
  [[nodiscard]] bool stop_requested() const;
  Bus& bus();

 private:
  std::unique_ptr<Memory> ram_;
  std::unique_ptr<Bus> bus_;
  std::atomic<bool> running_{false};
  std::atomic<bool> stop_requested_{false};
  std::atomic<bool> loop_active_{false};
  std::atomic<bool> wake_pending_{false};
  std::mutex wait_mutex_;
  std::condition_variable wait_cv_;
};

}  // namespace ai386
