#ifndef DRONE_SIMULATOR_CONFIG_NETWORK_CONFIG_H_
#define DRONE_SIMULATOR_CONFIG_NETWORK_CONFIG_H_

#include <cstdint>
#include <optional>
#include <string>

namespace drone_simulator::config {

enum class FecScheme { kNone, kXor };

struct NetworkConfig {
  std::string gcs_address;
  uint16_t command_port = 0;
  uint16_t telemetry_port = 0;
  uint16_t video_port = 0;
  FecScheme fec_scheme = FecScheme::kNone;
  uint32_t fec_group_size = 0;
  // Local OpenCV window of the rendered frames ([simulator] preview).
  bool preview = true;
  std::optional<double> command_timeout_s;
};

std::optional<NetworkConfig> LoadNetworkConfig(const std::string& path);

}

#endif
