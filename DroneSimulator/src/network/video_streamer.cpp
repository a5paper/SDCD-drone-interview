#include "network/video_streamer.h"

#include <algorithm>

#include "config/config.h"

namespace drone_simulator::network {

bool VideoStreamer::Open(std::string_view address, uint16_t port) {
  return sender_.Open(address, port);
}

void VideoStreamer::Send(std::span<const uint8_t> jpeg, uint64_t frame_id,
                         double timestamp_s) {
  constexpr size_t kPayload = config::kMaxChunkPayloadBytes;
  const size_t chunk_count = (jpeg.size() + kPayload - 1) / kPayload;

  chunk_.set_frame_id(frame_id);
  chunk_.set_timestamp_s(timestamp_s);
  chunk_.set_chunk_count(static_cast<uint32_t>(chunk_count));
  chunk_.set_frame_size(static_cast<uint32_t>(jpeg.size()));

  for (size_t i = 0; i < chunk_count; ++i) {
    const size_t offset = i * kPayload;
    chunk_.set_chunk_index(static_cast<uint32_t>(i));
    chunk_.set_payload(jpeg.data() + offset,
                       std::min(kPayload, jpeg.size() - offset));
    sender_.Send(chunk_);
  }
}

uint64_t VideoStreamer::GetFailedSends() const {
  return sender_.GetFailedSends();
}

}
