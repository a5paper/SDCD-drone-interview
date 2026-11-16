#include "network/message_sender.h"

#include <span>

namespace drone_simulator::network {

bool MessageSender::Open(std::string_view address, uint16_t port) {
  return socket_.Connect(address, port);
}

void MessageSender::Send(const google::protobuf::MessageLite& message) {
  if (!message.SerializeToString(&datagram_)) {
    ++failed_sends_;
    return;
  }

  const std::span<const uint8_t> data(
      reinterpret_cast<const uint8_t*>(datagram_.data()), datagram_.size());
  if (socket_.Send(data).status != SendStatus::kSent) {
    ++failed_sends_;
  }
}

uint64_t MessageSender::GetFailedSends() const { return failed_sends_; }

}
