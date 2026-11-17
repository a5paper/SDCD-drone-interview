#include "config/network_config.h"

#include <charconv>
#include <iostream>
#include <map>
#include <string>

#include "config/ini_file.h"

namespace drone_simulator::config {
namespace {

constexpr uint32_t kMaxPort = 65535;
constexpr uint32_t kMaxFecGroupSize = 1024;

std::optional<uint32_t> ParseNumber(const std::string& text, uint32_t min,
                                    uint32_t max) {
  const char* first = text.data();
  const char* last = first + text.size();
  uint32_t value = 0;
  const auto [end, error] = std::from_chars(first, last, value);
  if (error != std::errc() || end != last || value < min || value > max) {
    return std::nullopt;
  }
  return value;
}

std::optional<uint16_t> ParsePort(const std::string& text) {
  const std::optional<uint32_t> port = ParseNumber(text, 1, kMaxPort);
  if (!port) {
    return std::nullopt;
  }
  return static_cast<uint16_t>(*port);
}

std::optional<bool> ParseBool(const std::string& text) {
  const std::map<std::string, bool> values = {{"true", true}, {"false", false}};
  const auto it = values.find(text);
  if (it == values.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::optional<FecScheme> ParseFecScheme(const std::string& text) {
  const std::map<std::string, FecScheme> schemes = {{"none", FecScheme::kNone},
                                                    {"xor", FecScheme::kXor}};
  const auto it = schemes.find(text);
  if (it == schemes.end()) {
    return std::nullopt;
  }
  return it->second;
}

}

std::optional<NetworkConfig> LoadNetworkConfig(const std::string& path) {
  std::optional<IniValues> file = ReadIniFile(path);
  if (!file) {
    std::cerr << "Cannot read config file: " << path << std::endl;
    return std::nullopt;
  }
  IniValues& values = *file;

  NetworkConfig config;
  config.gcs_address = values["gcs_address"];
  const std::optional<uint16_t> command_port =
      ParsePort(values["command_port"]);
  const std::optional<uint16_t> telemetry_port =
      ParsePort(values["telemetry_port"]);
  const std::optional<uint16_t> video_port = ParsePort(values["video_port"]);

  if (config.gcs_address.empty() || !command_port || !telemetry_port ||
      !video_port) {
    std::cerr << path
              << ": needs gcs_address and command_port, telemetry_port, "
                 "video_port in 1-65535"
              << std::endl;
    return std::nullopt;
  }

  const std::optional<FecScheme> fec_scheme = ParseFecScheme(values["fec"]);
  const std::optional<uint32_t> fec_group_size =
      ParseNumber(values["fec_group_size"], 0, kMaxFecGroupSize);
  if (!fec_scheme || !fec_group_size) {
    std::cerr << path << ": needs fec = none or xor, and fec_group_size in 0-"
              << kMaxFecGroupSize << std::endl;
    return std::nullopt;
  }

  const std::optional<bool> preview = ParseBool(values["preview"]);
  if (!preview) {
    std::cerr << path << ": needs preview = true or false" << std::endl;
    return std::nullopt;
  }

  config.command_port = *command_port;
  config.telemetry_port = *telemetry_port;
  config.video_port = *video_port;
  config.fec_scheme = *fec_scheme;
  config.fec_group_size = *fec_group_size;
  config.preview = *preview;
  return config;
}

}
