#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "config/config.hpp"
#include "machine/machine.hpp"

namespace {

// The machine is owned by main; the signal handler only requests a stop,
// which is safe to call asynchronously.
ai386::Machine* g_machine = nullptr;

void HandleStopSignal(int) {
  if (g_machine != nullptr) g_machine->RequestStop();
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const std::vector<std::string> args(argv + 1, argv + argc);
    const ai386::Config config = ai386::ParseConfig(args);
    ai386::Machine machine(config);
    g_machine = &machine;
    std::signal(SIGINT, HandleStopSignal);
    std::signal(SIGTERM, HandleStopSignal);

    machine.Start();
    std::cout << "ai386: machine started with " << config.ram_bytes / (1024ULL * 1024ULL)
              << " MiB RAM\n";

    // The guest execution loop joins with the CPU stage; until then the CLI
    // keeps the machine alive until a stop request (Ctrl+C) arrives.
    while (!machine.stop_requested()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    machine.Stop();
    g_machine = nullptr;
    std::cout << "ai386: machine stopped\n";
    return 0;
  } catch (const ai386::ConfigError& error) {
    std::cerr << "config error: " << error.what() << '\n';
    return 1;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
