#include "machine/machine.hpp"

#include <chrono>
#include <stdexcept>

namespace ai386 {
namespace {

constexpr std::chrono::milliseconds kStopPollInterval{20};

}  // namespace

Machine::Machine(const Config& config) {
  ValidateConfig(config);
  ram_ = std::make_unique<Memory>(config.ram_bytes);
  bus_ = std::make_unique<Bus>(*ram_);
}

Machine::~Machine() { Stop(); }

void Machine::Start() {
  if (running_.load(std::memory_order_acquire)) return;
  for (Device* device : bus_->devices()) device->Start();
  running_.store(true, std::memory_order_release);
}

void Machine::Stop() {
  if (!running_.load(std::memory_order_acquire)) return;
  for (Device* device : bus_->devices()) device->Stop();
  running_.store(false, std::memory_order_release);
}

void Machine::Run(Cpu& cpu) {
  bool expected = false;
  if (!loop_active_.compare_exchange_strong(expected, true)) {
    throw std::logic_error("machine run loop is already active");
  }
  // A fresh loop must not inherit a stop request of the previous one.
  stop_requested_.store(false, std::memory_order_release);
  Start();
  try {
    while (!stop_requested_.load(std::memory_order_acquire)) {
      if (cpu.Step()) continue;
      // HLT: no instruction is executed until an interrupt wakes the CPU or
      // the stop is requested; the timed wait only polls for signal handlers.
      std::unique_lock<std::mutex> lock(wait_mutex_);
      while (!stop_requested_.load(std::memory_order_acquire) &&
             !wake_pending_.load(std::memory_order_acquire)) {
        wait_cv_.wait_for(lock, kStopPollInterval, [this] {
          return stop_requested_.load(std::memory_order_acquire) ||
                 wake_pending_.load(std::memory_order_acquire);
        });
      }
      wake_pending_.store(false, std::memory_order_release);
    }
  } catch (...) {
    loop_active_.store(false, std::memory_order_release);
    Stop();
    throw;
  }
  loop_active_.store(false, std::memory_order_release);
  Stop();
}

void Machine::RequestStop() {
  // Only an atomic store: this must stay safe to call from a signal handler,
  // so the run loop polls the flag instead of waiting on a notification.
  stop_requested_.store(true, std::memory_order_release);
}

void Machine::Wake() {
  wake_pending_.store(true, std::memory_order_release);
  wait_cv_.notify_all();
}

bool Machine::running() const { return running_.load(std::memory_order_acquire); }

bool Machine::stop_requested() const { return stop_requested_.load(std::memory_order_acquire); }

Bus& Machine::bus() {
  if (bus_ == nullptr) throw std::logic_error("machine is not initialized");
  return *bus_;
}

}  // namespace ai386
