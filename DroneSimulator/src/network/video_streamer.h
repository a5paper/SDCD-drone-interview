#ifndef DRONE_SIMULATOR_NETWORK_VIDEO_STREAMER_H_
#define DRONE_SIMULATOR_NETWORK_VIDEO_STREAMER_H_

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#include "network/udp_socket.h"
#include "protocols/video.pb.h"

namespace drone_simulator::network {

// Splits each JPEG frame into VideoChunk datagrams and sends them over UDP.
class VideoStreamer {
 public:
  [[nodiscard]] bool Open(std::string_view address, uint16_t port);
  void Send(std::span<const uint8_t> jpeg, uint64_t frame_id);
  uint64_t GetFailedSends() const;

 private:
  UdpSocket socket_;
  drone_protocol::VideoChunk chunk_;
  std::string datagram_;
  uint64_t failed_sends_ = 0;
};

}

#endif
