#include "config/network_config.h"

#include <charconv>
#include <iostream>
#include <string>

#include "config/ini_file.h"

namespace drone_simulator::config {
namespace {

constexpr int kMaxPort = 65535;

std::optional<uint16_t> ParsePort(const std::string& text) {
  int port = 0;
  const auto [end, error] =
      std::from_chars(text.data(), text.data() + text.size(), port);
  if (error != std::errc() || end != text.data() + text.size() || port < 1 ||
      port > kMaxPort) {
    return std::nullopt;
  }
  return static_cast<uint16_t>(port);
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

  config.command_port = *command_port;
  config.telemetry_port = *telemetry_port;
  config.video_port = *video_port;
  return config;
}

}
