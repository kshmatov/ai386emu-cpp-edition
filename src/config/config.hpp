#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ai386 {

// Built-in defaults of the emulator configuration.
inline constexpr std::uint64_t kDefaultRamBytes = 16ULL * 1024 * 1024;
inline constexpr std::uint64_t kDefaultHdSizeBytes = 1ULL << 30U;
inline constexpr std::size_t kMaxCdCount = 3;
inline constexpr std::uint64_t kDefaultClockHz = 100000000;
inline constexpr std::uint64_t kMaxClockHz = 1000000000;
inline constexpr std::uint64_t kFloppySizeBytes = 1474560;

// Launch configuration of the virtual machine: storage, memory, boot order.
struct Config {
  std::filesystem::path hd_path;
  std::uint64_t hd_size_bytes = kDefaultHdSizeBytes;
  std::vector<std::filesystem::path> cd_images;
  std::vector<std::filesystem::path> shared_dirs;
  std::uint64_t ram_bytes = kDefaultRamBytes;
  std::vector<std::string> boot_order{"hd"};
  std::filesystem::path floppy;
  std::uint64_t clock_hz = kDefaultClockHz;
  // Start time of the RTC calendar in seconds since the Unix epoch; when
  // empty, the host time at VM creation is used.
  std::optional<std::int64_t> rtc_start_epoch;
};

// Error of configuration parsing or validation with a human-readable message.
class ConfigError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

// Parses a size with an optional K/M/G suffix, e.g. "256M" -> 256 MiB.
std::uint64_t ParseSize(std::string_view text);

// Parses "host" as nullopt (host time at VM creation) or an RFC3339
// timestamp (e.g. "2024-02-28T23:59:59Z") as seconds since the Unix epoch.
// Throws ConfigError on a malformed value or a year outside 1900..2099.
std::optional<std::int64_t> ParseRtcStart(std::string_view text);

// Validates a configuration: storage paths, CD limit, RAM size and boot order.
// Throws ConfigError. Also called from Machine::Create, so a direct machine
// creation via API cannot bypass the checks.
void ValidateConfig(const Config& config);

// Parses command line arguments and merges them over an optional TOML file
// given with --config. Priority: command line > config file > defaults.
// Unknown TOML keys are ignored (they belong to later features); unknown
// command line parameters are rejected. Throws ConfigError.
Config ParseConfig(const std::vector<std::string>& args);

}  // namespace ai386
