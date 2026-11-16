#ifndef DRONE_SIMULATOR_NETWORK_COMMAND_RECEIVER_H_
#define DRONE_SIMULATOR_NETWORK_COMMAND_RECEIVER_H_

#include <array>
#include <cstdint>
#include <optional>

#include "core/config.h"
#include "network/udp_socket.h"
#include "protocols/commands.pb.h"

namespace drone_simulator::network {

class CommandReceiver {
 public:
  [[nodiscard]] bool Open(uint16_t port);

  std::optional<drone_protocol::VelocityChangeCommand> Poll();

 private:
  UdpSocket socket_;
  std::array<uint8_t, config::kMtuSize> buffer_{};
};

}

#endif
