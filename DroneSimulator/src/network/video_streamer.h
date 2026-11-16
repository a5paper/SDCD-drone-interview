#ifndef DRONE_SIMULATOR_NETWORK_VIDEO_STREAMER_H_
#define DRONE_SIMULATOR_NETWORK_VIDEO_STREAMER_H_

#include <cstdint>
#include <span>
#include <string_view>

#include "network/message_sender.h"
#include "protocols/video.pb.h"

namespace drone_simulator::network {

class VideoStreamer {
 public:
  [[nodiscard]] bool Open(std::string_view address, uint16_t port);
  void Send(std::span<const uint8_t> jpeg, uint64_t frame_id);
  uint64_t GetFailedSends() const;

 private:
  MessageSender sender_;
  drone_protocol::VideoChunk chunk_;
};

}

#endif
