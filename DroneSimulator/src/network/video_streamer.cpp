#include "network/video_streamer.h"

#include <algorithm>

#include "core/config.h"

namespace drone_simulator::network {

bool VideoStreamer::Open(std::string_view address, uint16_t port) {
  return socket_.Connect(address, port);
}

void VideoStreamer::Send(std::span<const uint8_t> jpeg, uint64_t frame_id) {
  const size_t chunk_count =
      (jpeg.size() + config::kMaxChunkPayload - 1) / config::kMaxChunkPayload;

  chunk_.set_frame_id(frame_id);
  chunk_.set_chunk_count(static_cast<uint32_t>(chunk_count));
  chunk_.set_frame_size(static_cast<uint32_t>(jpeg.size()));

  for (size_t i = 0; i < chunk_count; ++i) {
    const size_t offset = i * config::kMaxChunkPayload;
    const size_t size =
        std::min(config::kMaxChunkPayload, jpeg.size() - offset);

    chunk_.set_chunk_index(static_cast<uint32_t>(i));
    chunk_.set_payload(jpeg.data() + offset, size);
    chunk_.SerializeToString(&datagram_);

    const SendResult result = socket_.Send(
        {reinterpret_cast<const uint8_t*>(datagram_.data()), datagram_.size()});
    if (result.status != SendStatus::kSent) {
      ++failed_sends_;
    }
  }
}

uint64_t VideoStreamer::GetFailedSends() const { return failed_sends_; }

}
