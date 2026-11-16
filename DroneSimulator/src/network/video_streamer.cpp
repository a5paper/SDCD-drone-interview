#include "network/video_streamer.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "config/config.h"

namespace drone_simulator::network {

VideoStreamer::VideoStreamer(std::unique_ptr<FecStrategy> fec)
    : fec_(std::move(fec)) {}

bool VideoStreamer::Open(std::string_view address, uint16_t port) {
  return sender_.Open(address, port);
}

void VideoStreamer::Send(std::span<const uint8_t> jpeg, uint64_t frame_id,
                         double timestamp_s) {
  constexpr size_t kPayload = config::kMaxChunkPayloadBytes;
  chunks_.clear();
  for (size_t offset = 0; offset < jpeg.size(); offset += kPayload) {
    chunks_.push_back(
        jpeg.subspan(offset, std::min(kPayload, jpeg.size() - offset)));
  }

  chunk_.set_frame_id(frame_id);
  chunk_.set_timestamp_s(timestamp_s);
  chunk_.set_chunk_count(static_cast<uint32_t>(chunks_.size()));
  chunk_.set_frame_size(static_cast<uint32_t>(jpeg.size()));
  chunk_.set_fec_scheme(fec_->GetScheme());
  chunk_.set_fec_group_size(fec_->GetGroupSize(chunks_.size()));

  for (size_t i = 0; i < chunks_.size(); ++i) {
    chunk_.set_chunk_index(static_cast<uint32_t>(i));
    chunk_.set_payload(chunks_[i].data(), chunks_[i].size());
    sender_.Send(chunk_);
  }

  const std::vector<std::string> parity = fec_->MakeParity(chunks_);
  for (size_t group = 0; group < parity.size(); ++group) {
    chunk_.set_chunk_index(static_cast<uint32_t>(chunks_.size() + group));
    chunk_.set_payload(parity[group]);
    sender_.Send(chunk_);
  }
}

uint64_t VideoStreamer::GetFailedSends() const {
  return sender_.GetFailedSends();
}

}
