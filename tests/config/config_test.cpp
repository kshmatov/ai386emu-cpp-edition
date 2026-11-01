#include "config/config.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace ai386 {
namespace {

class ConfigTest : public ::testing::Test {
 protected:
  void SetUp() override {
    test_dir_ = std::filesystem::temp_directory_path() / "ai386-config-test";
    std::filesystem::create_directories(test_dir_);
  }
  void TearDown() override { std::filesystem::remove_all(test_dir_); }

  std::filesystem::path WriteFile(const std::string& name, const std::string& content) {
    const std::filesystem::path path = test_dir_ / name;
    std::ofstream stream(path);
    stream << content;
    return path;
  }

  std::filesystem::path test_dir_;
};

// Scenario: Указание размера HD
TEST_F(ConfigTest, ParsesSizeSuffixes) {
  EXPECT_EQ(ParseSize("1024"), 1024u);
  EXPECT_EQ(ParseSize("256K"), 256u * 1024);
  EXPECT_EQ(ParseSize("16M"), 16u * 1024 * 1024);
  EXPECT_EQ(ParseSize("1G"), 1024u * 1024 * 1024);
  EXPECT_THROW(ParseSize("abc"), ConfigError);
  EXPECT_THROW(ParseSize(""), ConfigError);
  EXPECT_THROW(ParseSize("12X"), ConfigError);
}

// Scenario: Использование существующего файла / Указание пути HD
TEST_F(ConfigTest, ParsesHdPathAndSize) {
  const Config config = ParseConfig({"--hd", "disk.img", "--hd-size", "2G"});
  EXPECT_EQ(config.hd_path, std::filesystem::path("disk.img"));
  EXPECT_EQ(config.hd_size_bytes, 2u * 1024 * 1024 * 1024);
}

// Scenario: Значение по умолчанию
TEST_F(ConfigTest, DefaultsApplyWhenNotSpecified) {
  const Config config = ParseConfig({"--hd", "disk.img"});
  EXPECT_EQ(config.ram_bytes, kDefaultRamBytes);
  EXPECT_EQ(config.hd_size_bytes, kDefaultHdSizeBytes);
  EXPECT_EQ(config.boot_order, std::vector<std::string>({"hd"}));
}

// Scenario: Подключение нескольких CD
TEST_F(ConfigTest, MultipleCdAndSharedPathsAccumulate) {
  const std::filesystem::path iso1 = WriteFile("one.iso", "x");
  const std::filesystem::path iso2 = WriteFile("two.iso", "x");
  const std::filesystem::path dir = test_dir_ / "shared";
  std::filesystem::create_directories(dir);
  const Config config = ParseConfig(
      {"--hd", "disk.img", "--cd", iso1.string(), "--cd", iso2.string(), "--shared", dir.string()});
  EXPECT_EQ(config.cd_images.size(), 2u);
  EXPECT_EQ(config.shared_dirs.size(), 1u);
}

// Scenario: CD-путь без файла
TEST_F(ConfigTest, MissingCdImageRejected) {
  const std::vector<std::string> args = {"--hd", "disk.img", "--cd",
                                         (test_dir_ / "missing.iso").string()};
  EXPECT_THROW(ParseConfig(args), ConfigError);
}

// Scenario: CD-путь без каталога
TEST_F(ConfigTest, MissingSharedDirRejected) {
  const std::vector<std::string> args = {"--hd", "disk.img", "--shared",
                                         (test_dir_ / "missing-dir").string()};
  EXPECT_THROW(ParseConfig(args), ConfigError);
}

// Scenario: Превышение лимита — более трёх CD суммарно.
TEST_F(ConfigTest, MoreThanThreeCdRejected) {
  const std::filesystem::path iso = WriteFile("a.iso", "x");
  const std::filesystem::path dir = test_dir_ / "d";
  std::filesystem::create_directories(dir);
  const std::vector<std::string> args = {"--hd",     "disk.img",   "--cd", iso.string(),
                                         "--cd",     iso.string(), "--cd", iso.string(),
                                         "--shared", dir.string()};
  EXPECT_THROW(ParseConfig(args), ConfigError);
}

// Scenario: Изменение объёма памяти
TEST_F(ConfigTest, RamSizeParsed) {
  const Config config = ParseConfig({"--hd", "disk.img", "--ram", "64M"});
  EXPECT_EQ(config.ram_bytes, 64u * 1024 * 1024);
}

// Scenario: Изменение порядка загрузки
TEST_F(ConfigTest, BootOrderPreserved) {
  const Config config = ParseConfig({"--hd", "disk.img", "--boot", "cd,hd"});
  EXPECT_EQ(config.boot_order, std::vector<std::string>({"cd", "hd"}));
}

// Scenario: Недопустимое устройство в очереди
TEST_F(ConfigTest, UnsupportedBootDeviceRejected) {
  const std::vector<std::string> args = {"--hd", "disk.img", "--boot", "usb"};
  EXPECT_THROW(ParseConfig(args), ConfigError);
}

// Scenario: Запуск с файлом конфигурации
TEST_F(ConfigTest, ValuesLoadedFromFile) {
  const std::filesystem::path path =
      WriteFile("config.toml", "hd = \"disk.img\"\nram = \"32M\"\nboot = [\"hd\", \"cd\"]\n");
  const Config config = ParseConfig({"--config", path.string()});
  EXPECT_EQ(config.hd_path, std::filesystem::path("disk.img"));
  EXPECT_EQ(config.ram_bytes, 32u * 1024 * 1024);
  EXPECT_EQ(config.boot_order, std::vector<std::string>({"hd", "cd"}));
}

// Scenario: Приоритет командной строки
TEST_F(ConfigTest, CommandLineOverridesFile) {
  const std::filesystem::path path = WriteFile("config.toml", "hd = \"disk.img\"\nram = \"32M\"\n");
  const Config config = ParseConfig({"--config", path.string(), "--ram", "64M"});
  EXPECT_EQ(config.ram_bytes, 64u * 1024 * 1024);
  EXPECT_EQ(config.hd_path, std::filesystem::path("disk.img"));
}

// Scenario: Недопустимый файл конфигурации
TEST_F(ConfigTest, MissingConfigFileRejected) {
  const std::vector<std::string> args = {"--config", (test_dir_ / "absent.toml").string()};
  EXPECT_THROW(ParseConfig(args), ConfigError);
}

// Scenario: Недопустимый файл конфигурации (invalid TOML)
TEST_F(ConfigTest, InvalidTomlRejected) {
  const std::filesystem::path path = WriteFile("broken.toml", "hd = [unterminated\n");
  const std::vector<std::string> args = {"--config", path.string()};
  EXPECT_THROW(ParseConfig(args), ConfigError);
}

TEST_F(ConfigTest, HdPathIsRequired) { EXPECT_THROW(ParseConfig({"--ram", "16M"}), ConfigError); }

TEST_F(ConfigTest, UnknownParameterRejected) {
  const std::vector<std::string> args = {"--hd", "disk.img", "--bogus", "1"};
  EXPECT_THROW(ParseConfig(args), ConfigError);
}

// Scenario: Настройка частоты (clock_hz: TOML + CLI с приоритетом)
TEST_F(ConfigTest, ClockHzPriorityCliOverToml) {
  const std::filesystem::path path =
      WriteFile("clock.toml", "hd = \"disk.img\"\nclock_hz = 50000000\n");
  const Config config = ParseConfig({"--config", path.string(), "--clock-hz", "100000000"});
  EXPECT_EQ(config.clock_hz, 100000000u);
}

// Scenario: Недопустимая частота
TEST_F(ConfigTest, InvalidClockHzRejected) {
  EXPECT_THROW(ParseConfig({"--hd", "disk.img", "--clock-hz", "0"}), ConfigError);
  EXPECT_THROW(ParseConfig({"--hd", "disk.img", "--clock-hz", "-5"}), ConfigError);
  EXPECT_THROW(ParseConfig({"--hd", "disk.img", "--clock-hz", "1000000001"}), ConfigError);
}

// Scenario: Воспроизводимое начало (rtc_start)
TEST_F(ConfigTest, RtcStartFixedDate) {
  const Config config = ParseConfig({"--hd", "disk.img", "--rtc-start", "2024-02-28T23:59:59Z"});
  ASSERT_TRUE(config.rtc_start_epoch.has_value());
  EXPECT_EQ(*config.rtc_start_epoch, 1709164799);
}

// Scenario: Переопределение CLI (host сбрасывает дату из TOML)
TEST_F(ConfigTest, RtcStartHostOverridesFile) {
  const std::filesystem::path path =
      WriteFile("rtc.toml", "hd = \"disk.img\"\nrtc_start = \"2024-02-28T23:59:59Z\"\n");
  const Config config = ParseConfig({"--config", path.string(), "--rtc-start", "host"});
  EXPECT_FALSE(config.rtc_start_epoch.has_value());
}

// Scenario: Недопустимые значения rtc_start
TEST_F(ConfigTest, RtcStartInvalidRejected) {
  EXPECT_THROW(ParseConfig({"--hd", "disk.img", "--rtc-start", "2024-02-30T00:00:00Z"}),
               ConfigError);
  EXPECT_THROW(ParseConfig({"--hd", "disk.img", "--rtc-start", "1899-01-01T00:00:00Z"}),
               ConfigError);
  EXPECT_THROW(ParseConfig({"--hd", "disk.img", "--rtc-start", "not-a-date"}), ConfigError);
}

// Scenario: Подключение floppy-образа / неверный размер
TEST_F(ConfigTest, FloppySizeValidated) {
  const std::filesystem::path floppy = test_dir_ / "disk.flp";
  std::ofstream stream(floppy, std::ios::binary);
  stream << std::string(1474560, '\0');
  stream.close();
  const Config config = ParseConfig({"--hd", "disk.img", "--floppy", floppy.string()});
  EXPECT_EQ(config.floppy, floppy);

  const std::filesystem::path small = test_dir_ / "small.flp";
  std::ofstream small_stream(small, std::ios::binary);
  small_stream << "tiny";
  small_stream.close();
  EXPECT_THROW(ParseConfig({"--hd", "disk.img", "--floppy", small.string()}), ConfigError);
}

// Scenario: Floppy без файла — дисковод не подключается
TEST_F(ConfigTest, FloppyAbsentMeansNoDrive) {
  const Config config = ParseConfig({"--hd", "disk.img"});
  EXPECT_TRUE(config.floppy.empty());
}

TEST_F(ConfigTest, KeyValueFormIsAccepted) {
  const Config config = ParseConfig({"--hd=disk.img", "--ram=8M"});
  EXPECT_EQ(config.hd_path, std::filesystem::path("disk.img"));
  EXPECT_EQ(config.ram_bytes, 8u * 1024 * 1024);
}

}  // namespace
}  // namespace ai386
