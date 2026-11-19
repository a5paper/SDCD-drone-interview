#include "network/video_streamer.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "config/config.h"
#include "network/udp_socket.h"

namespace drone_simulator::network {
namespace {

// Away from the simulator's ports (5600-5602), so a running one is untouched.
constexpr uint16_t kTestPort = 5699;
constexpr char kLoopback[] = "127.0.0.1";
constexpr size_t kIpUdpHeaderBytes = 28;
constexpr size_t kFrameBytes = 2 * config::kMaxChunkPayloadBytes + 200;
constexpr size_t kDataChunks = 3;
static_assert(kDataChunks == kFrameBytes / config::kMaxChunkPayloadBytes + 1);
constexpr uint32_t kGroupSize = 2;
constexpr size_t kParityChunks = 2;
static_assert(kParityChunks == (kDataChunks + kGroupSize - 1) / kGroupSize);
constexpr size_t kExactChunks = 2;
constexpr auto kExactFrameBytes = kExactChunks * config::kMaxChunkPayloadBytes;
constexpr uint64_t kFrameId = 42;
constexpr double kTimestampS = 1.5;

class VideoStreamerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_TRUE(receiver_.Bind(kTestPort));
    ASSERT_TRUE(receiver_.SetNonBlocking());
    for (size_t i = 0; i < frame_.size(); ++i) {
      frame_[i] = static_cast<uint8_t>(i);
    }
  }

  // Streams frame with fec and returns the datagrams that reached the port.
  std::vector<drone_protocol::VideoChunk> Stream(
      std::unique_ptr<FecStrategy> fec, std::span<const uint8_t> frame) {
    VideoStreamer streamer(std::move(fec));
    EXPECT_TRUE(streamer.Open(kLoopback, kTestPort));
    streamer.Send(frame, kFrameId, kTimestampS);
    EXPECT_EQ(streamer.GetFailedSends(), 0u);

    std::vector<drone_protocol::VideoChunk> chunks;
    std::array<uint8_t, config::kMtuBytes> buffer;
    for (ReceiveResult result = receiver_.Receive(buffer);
         result.status == ReceiveStatus::kData;
         result = receiver_.Receive(buffer)) {
      EXPECT_LE(result.bytes, config::kMtuBytes - kIpUdpHeaderBytes);
      const int size = static_cast<int>(result.bytes);
      drone_protocol::VideoChunk& chunk = chunks.emplace_back();
      EXPECT_TRUE(chunk.ParseFromArray(buffer.data(), size));
    }
    return chunks;
  }

  UdpSocket receiver_;
  std::array<uint8_t, kFrameBytes> frame_;
};

TEST_F(VideoStreamerTest, SplitsFrameIntoOrderedChunks) {
  const std::vector<drone_protocol::VideoChunk> chunks =
      Stream(std::make_unique<NoFec>(), frame_);
  ASSERT_EQ(chunks.size(), kDataChunks);

  std::string payload;
  for (size_t i = 0; i < chunks.size(); ++i) {
    EXPECT_EQ(chunks[i].frame_id(), kFrameId);
    EXPECT_EQ(chunks[i].timestamp_s(), kTimestampS);
    EXPECT_EQ(chunks[i].chunk_index(), i);
    EXPECT_EQ(chunks[i].chunk_count(), kDataChunks);
    EXPECT_EQ(chunks[i].frame_size(), kFrameBytes);
    EXPECT_EQ(chunks[i].fec_scheme(), drone_protocol::FEC_SCHEME_NONE);
    EXPECT_EQ(chunks[i].fec_group_size(), 0u);
    payload += chunks[i].payload();
  }
  EXPECT_EQ(payload, std::string(frame_.begin(), frame_.end()));
}

TEST_F(VideoStreamerTest, AppendsParityAfterDataChunks) {
  const std::vector<drone_protocol::VideoChunk> chunks =
      Stream(std::make_unique<XorFec>(kGroupSize), frame_);
  ASSERT_EQ(chunks.size(), kDataChunks + kParityChunks);
  for (size_t i = 0; i < chunks.size(); ++i) {
    EXPECT_EQ(chunks[i].chunk_index(), i);
    EXPECT_EQ(chunks[i].chunk_count(), kDataChunks);
    EXPECT_EQ(chunks[i].fec_scheme(), drone_protocol::FEC_SCHEME_XOR);
    EXPECT_EQ(chunks[i].fec_group_size(), kGroupSize);
  }
  std::string first_group = chunks[0].payload();
  for (size_t byte = 0; byte < first_group.size(); ++byte) {
    first_group[byte] ^= chunks[1].payload()[byte];
  }
  EXPECT_EQ(chunks[kDataChunks].payload(), first_group);
  EXPECT_EQ(chunks[kDataChunks + 1].payload(), chunks[2].payload());
}

TEST_F(VideoStreamerTest, ExactMultipleOfPayloadHasNoShortChunk) {
  const std::span<const uint8_t> frame(frame_.data(), kExactFrameBytes);
  const std::vector<drone_protocol::VideoChunk> chunks =
      Stream(std::make_unique<NoFec>(), frame);
  ASSERT_EQ(chunks.size(), kExactChunks);
  for (const drone_protocol::VideoChunk& chunk : chunks) {
    EXPECT_EQ(chunk.chunk_count(), kExactChunks);
    EXPECT_EQ(chunk.payload().size(), config::kMaxChunkPayloadBytes);
  }
}

TEST_F(VideoStreamerTest, EmptyFrameSendsNothing) {
  EXPECT_TRUE(Stream(std::make_unique<XorFec>(0), {}).empty());
}

}
}
