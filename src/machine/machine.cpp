#include "machine/machine.hpp"

#include <chrono>
#include <stdexcept>

namespace ai386 {
namespace {

constexpr std::chrono::milliseconds kStopPollInterval{20};

std::int64_t CurrentUnixTime() {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace

Machine::Machine(const Config& config)
    : clock_(config.clock_hz), rtc_(pic_, config.rtc_start_epoch.value_or(CurrentUnixTime())) {
  ValidateConfig(config);
  ram_ = std::make_unique<Memory>(config.ram_bytes);
  bus_ = std::make_unique<Bus>(*ram_);
  bus_->AttachPort(pic_, 0x20, 2);
  bus_->AttachPort(pic_, 0xA0, 2);
  bus_->AttachPort(pit_, 0x40, 4);
  bus_->AttachPort(pit_, 0x61, 1);
  bus_->AttachPort(rtc_, 0x70, 2);
  rtc_.InitCmos(config.ram_bytes, !config.floppy.empty(), true);
}

Machine::~Machine() { Stop(); }

void Machine::AdvanceDevices(std::uint64_t cycles) {
  const std::uint64_t nanos_before = clock_.nanos();
  clock_.Advance(cycles);
  pit_.Advance(cycles, clock_.clock_hz());
  rtc_.Advance(clock_.nanos() - nanos_before);
}

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
      if (cpu.Step()) {
        // One instruction advances virtual time by one cycle at least;
        // devices follow the shared clock.
        AdvanceDevices(1);
        continue;
      }
      // HLT: no instruction is executed until an interrupt wakes the CPU or
      // the stop is requested. Virtual time keeps flowing so timers can fire,
      // and the timed wait polls for signal handlers.
      std::unique_lock<std::mutex> lock(wait_mutex_);
      while (!stop_requested_.load(std::memory_order_acquire) &&
             !wake_pending_.load(std::memory_order_acquire) && !pic_.PendingVector().has_value()) {
        wait_cv_.wait_for(lock, kStopPollInterval, [this] {
          return stop_requested_.load(std::memory_order_acquire) ||
                 wake_pending_.load(std::memory_order_acquire);
        });
        // Idle ticks: devices advance by the wall interval of the poll.
        AdvanceDevices(clock_.clock_hz() * kStopPollInterval.count() / 1000);
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

VirtualClock& Machine::clock() { return clock_; }

Pic& Machine::pic() { return pic_; }

Pit& Machine::pit() { return pit_; }

Rtc& Machine::rtc() { return rtc_; }

Bus& Machine::bus() {
  if (bus_ == nullptr) throw std::logic_error("machine is not initialized");
  return *bus_;
}

}  // namespace ai386
