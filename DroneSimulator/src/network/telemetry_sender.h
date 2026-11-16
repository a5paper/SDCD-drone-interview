#ifndef DRONE_SIMULATOR_NETWORK_TELEMETRY_SENDER_H_
#define DRONE_SIMULATOR_NETWORK_TELEMETRY_SENDER_H_

#include <cstdint>
#include <string>
#include <string_view>

#include "network/udp_socket.h"
#include "protocols/telemetry.pb.h"

namespace drone_simulator::network {

class TelemetrySender {
 public:
  [[nodiscard]] bool Open(std::string_view address, uint16_t port);
  void Send(const drone_protocol::Telemetry& telemetry);
  uint64_t GetFailedSends() const;

 private:
  UdpSocket socket_;
  std::string datagram_;
  uint64_t failed_sends_ = 0;
};

}

#endif
