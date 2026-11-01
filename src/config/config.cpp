#include "config/config.hpp"

#include <charconv>
#include <optional>
#include <sstream>
#include <toml++/toml.hpp>

namespace ai386 {
namespace {

constexpr std::uint64_t kKilobyte = 1024U;
constexpr std::uint64_t kMegabyte = 1024ULL * 1024ULL;
constexpr std::uint64_t kGigabyte = 1024ULL * 1024U * 1024U;
constexpr std::uint64_t kAddressSpaceBytes = 4ULL * 1024U * 1024U * 1024U;

// Splits a comma-separated device list, e.g. "cd,hd" -> {"cd", "hd"}.
std::vector<std::string> SplitDevices(std::string_view text) {
  std::vector<std::string> devices;
  std::string current;
  for (const char ch : text) {
    if (ch == ',') {
      if (!current.empty()) devices.push_back(current);
      current.clear();
    } else if (ch != ' ') {
      current.push_back(ch);
    }
  }
  if (!current.empty()) devices.push_back(current);
  return devices;
}

bool IsBootDeviceSupported(const std::string& device) {
  return device == "hd" || device == "cd" || device == "floppy";
}

// Reads an optional string value from the TOML table.
std::optional<std::string> TomlString(const toml::table& table, std::string_view key) {
  const toml::node* node = table.get(key);
  if (node == nullptr) return std::nullopt;
  const toml::value<std::string>* value = node->as_string();
  if (value == nullptr) {
    throw ConfigError("config key '" + std::string(key) + "' must be a string");
  }
  return value->get();
}

// Reads an optional array-of-strings value from the TOML table.
std::optional<std::vector<std::string>> TomlStringArray(const toml::table& table,
                                                        std::string_view key) {
  const toml::node* node = table.get(key);
  if (node == nullptr) return std::nullopt;
  const toml::array* array = node->as_array();
  if (array == nullptr) {
    throw ConfigError("config key '" + std::string(key) + "' must be an array");
  }
  std::vector<std::string> values;
  for (const toml::node& item : *array) {
    const toml::value<std::string>* value = item.as_string();
    if (value == nullptr) {
      throw ConfigError("config key '" + std::string(key) + "' must contain only strings");
    }
    values.push_back(value->get());
  }
  return values;
}

// Loads a configuration file; missing file and invalid TOML are rejected.
toml::table LoadConfigFile(const std::filesystem::path& path) {
  if (!std::filesystem::is_regular_file(path)) {
    throw ConfigError("config file not found: " + path.string());
  }
  try {
    return toml::parse_file(path.string());
  } catch (const toml::parse_error& error) {
    std::ostringstream message;
    message << "invalid TOML in " << path.string() << ": " << error.description();
    throw ConfigError(message.str());
  }
}

}  // namespace

std::uint64_t ParseSize(std::string_view text) {
  if (text.empty()) throw ConfigError("empty size");
  std::string_view digits = text;
  std::uint64_t multiplier = 1;
  switch (text.back()) {
    case 'K':
    case 'k':
      multiplier = kKilobyte;
      digits = text.substr(0, text.size() - 1);
      break;
    case 'M':
    case 'm':
      multiplier = kMegabyte;
      digits = text.substr(0, text.size() - 1);
      break;
    case 'G':
    case 'g':
      multiplier = kGigabyte;
      digits = text.substr(0, text.size() - 1);
      break;
    default:
      break;
  }
  std::uint64_t value = 0;
  const auto [end, error] = std::from_chars(digits.data(), digits.data() + digits.size(), value);
  if (error != std::errc{} || end != digits.data() + digits.size()) {
    throw ConfigError("invalid size: " + std::string(text));
  }
  return value * multiplier;
}

void ValidateConfig(const Config& config) {
  if (config.hd_path.empty()) {
    throw ConfigError("hard disk image path is required (--hd)");
  }
  if (config.ram_bytes == 0) {
    throw ConfigError("RAM size must be positive");
  }
  if (config.ram_bytes > kAddressSpaceBytes) {
    throw ConfigError("RAM size must not exceed the 32-bit address space (4 GiB)");
  }
  const std::size_t cd_count = config.cd_images.size() + config.shared_dirs.size();
  if (cd_count > kMaxCdCount) {
    throw ConfigError("at most 3 CD devices are supported, got " + std::to_string(cd_count) +
                      " (rejected before HD creation and VM start)");
  }
  for (const std::filesystem::path& path : config.cd_images) {
    if (!std::filesystem::is_regular_file(path)) {
      throw ConfigError("CD image not found: " + path.string());
    }
  }
  for (const std::filesystem::path& path : config.shared_dirs) {
    if (!std::filesystem::is_directory(path)) {
      throw ConfigError("shared directory not found: " + path.string());
    }
  }
  if (config.boot_order.empty()) {
    throw ConfigError("boot order must not be empty");
  }
  for (const std::string& device : config.boot_order) {
    if (!IsBootDeviceSupported(device)) {
      throw ConfigError("unsupported boot device: " + device + " (expected hd, cd or floppy)");
    }
  }
}

Config ParseConfig(const std::vector<std::string>& args) {
  // First pass: find --config so the file layer is known before merging.
  std::optional<std::filesystem::path> config_path;
  for (std::size_t i = 0; i < args.size(); ++i) {
    const std::string& arg = args[i];
    std::string key = arg;
    std::string value;
    const std::size_t equals = arg.find('=');
    if (equals != std::string::npos) {
      key = arg.substr(0, equals);
      value = arg.substr(equals + 1);
    }
    if (key != "--config") continue;
    if (value.empty()) {
      if (i + 1 >= args.size()) throw ConfigError("--config requires a path");
      value = args[++i];
    }
    config_path = std::filesystem::path(value);
  }

  // Layer 1: defaults, layer 2: config file.
  Config config;
  if (config_path.has_value()) {
    const toml::table table = LoadConfigFile(*config_path);
    if (const std::optional<std::string> hd = TomlString(table, "hd")) {
      config.hd_path = std::filesystem::path(*hd);
    }
    if (const std::optional<std::string> size = TomlString(table, "hd_size")) {
      config.hd_size_bytes = ParseSize(*size);
    }
    if (const std::optional<std::vector<std::string>> cd = TomlStringArray(table, "cd")) {
      config.cd_images.clear();
      for (const std::string& path : *cd) config.cd_images.emplace_back(path);
    }
    if (const std::optional<std::vector<std::string>> shared = TomlStringArray(table, "shared")) {
      config.shared_dirs.clear();
      for (const std::string& path : *shared) config.shared_dirs.emplace_back(path);
    }
    if (const std::optional<std::string> ram = TomlString(table, "ram")) {
      config.ram_bytes = ParseSize(*ram);
    }
    if (const std::optional<std::vector<std::string>> boot = TomlStringArray(table, "boot")) {
      config.boot_order = *boot;
    }
    // Unknown keys are ignored: they belong to later features.
  }

  // Layer 3: command line, the highest priority. Repeated --cd/--shared
  // accumulate and, if present, replace the file layer lists entirely.
  std::vector<std::filesystem::path> cli_cd;
  std::vector<std::filesystem::path> cli_shared;
  for (std::size_t i = 0; i < args.size(); ++i) {
    std::string key = args[i];
    std::string value;
    bool has_value = false;
    const std::size_t equals = key.find('=');
    if (equals != std::string::npos) {
      value = key.substr(equals + 1);
      key = key.substr(0, equals);
      has_value = true;
    }
    if (key == "--config") {
      if (!has_value) ++i;  // value already consumed in the first pass
      continue;
    }
    if (!has_value) {
      if (i + 1 >= args.size()) throw ConfigError(key + " requires a value");
      value = args[++i];
    }
    if (key == "--hd") {
      config.hd_path = std::filesystem::path(value);
    } else if (key == "--hd-size") {
      config.hd_size_bytes = ParseSize(value);
    } else if (key == "--cd" || key == "--shared") {
      std::vector<std::filesystem::path>& target = (key == "--cd") ? cli_cd : cli_shared;
      target.emplace_back(value);
    } else if (key == "--ram") {
      config.ram_bytes = ParseSize(value);
    } else if (key == "--boot") {
      config.boot_order = SplitDevices(value);
    } else {
      throw ConfigError("unknown command line parameter: " + key);
    }
  }
  if (!cli_cd.empty()) config.cd_images = std::move(cli_cd);
  if (!cli_shared.empty()) config.shared_dirs = std::move(cli_shared);

  ValidateConfig(config);
  return config;
}

}  // namespace ai386
