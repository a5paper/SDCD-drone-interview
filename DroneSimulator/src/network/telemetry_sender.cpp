#include "network/telemetry_sender.h"

#include <span>
#include <string>

namespace drone_simulator::network {

bool TelemetrySender::Open(std::string_view address, uint16_t port) {
  return socket_.Connect(address, port);
}

void TelemetrySender::Send(const drone_protocol::Telemetry& telemetry) {
  if (!telemetry.SerializeToString(&datagram_)) {
    ++failed_sends_;
    return;
  }

  const std::span<const uint8_t> data(
      reinterpret_cast<const uint8_t*>(datagram_.data()), datagram_.size());
  if (socket_.Send(data).status != SendStatus::kSent) {
    ++failed_sends_;
  }
}

uint64_t TelemetrySender::GetFailedSends() const { return failed_sends_; }

}
