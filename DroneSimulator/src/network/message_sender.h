#ifndef DRONE_SIMULATOR_NETWORK_MESSAGE_SENDER_H_
#define DRONE_SIMULATOR_NETWORK_MESSAGE_SENDER_H_

#include <google/protobuf/message_lite.h>

#include <cstdint>
#include <string>
#include <string_view>

#include "network/udp_socket.h"

namespace drone_simulator::network {

class MessageSender {
 public:
  [[nodiscard]] bool Open(std::string_view address, uint16_t port);
  void Send(const google::protobuf::MessageLite& message);
  uint64_t GetFailedSends() const;

 private:
  UdpSocket socket_;
  std::string datagram_;
  uint64_t failed_sends_ = 0;
};

}

#endif
