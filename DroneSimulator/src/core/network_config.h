#ifndef DRONE_SIMULATOR_CORE_NETWORK_CONFIG_H_
#define DRONE_SIMULATOR_CORE_NETWORK_CONFIG_H_

#include <cstdint>
#include <optional>
#include <string>

namespace drone_simulator::config {

struct NetworkConfig {
  std::string gcs_address;
  uint16_t command_port = 0;
  uint16_t telemetry_port = 0;
  uint16_t video_port = 0;
};

std::optional<NetworkConfig> LoadNetworkConfig(const std::string& path);

}

#endif
