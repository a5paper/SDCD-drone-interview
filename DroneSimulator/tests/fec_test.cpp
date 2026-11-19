#include "network/fec.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

namespace drone_simulator::network {
namespace {

// Three data chunks; the last is shorter, as the last chunk of a frame is.
const std::vector<std::vector<uint8_t>> kData{
    {0x01, 0x02, 0x03, 0x04},
    {0xF0, 0x0F, 0xAA, 0x55},
    {0x11, 0x22},
};

Chunks MakeChunks() { return Chunks(kData.begin(), kData.end()); }

TEST(NoFecTest, MakesNoParity) {
  const NoFec fec;
  EXPECT_EQ(fec.GetScheme(), drone_protocol::FEC_SCHEME_NONE);
  EXPECT_EQ(fec.GetGroupSize(kData.size()), 0u);
  EXPECT_TRUE(fec.MakeParity(MakeChunks()).empty());
}

TEST(XorFecTest, GroupSizeZeroMeansWholeFrame) {
  const XorFec fec(0);
  EXPECT_EQ(fec.GetScheme(), drone_protocol::FEC_SCHEME_XOR);
  EXPECT_EQ(fec.GetGroupSize(kData.size()), kData.size());
  EXPECT_EQ(fec.MakeParity(MakeChunks()).size(), 1u);
}

TEST(XorFecTest, ParityIsZeroPaddedXorOfGroup) {
  const std::vector<std::string> parity = XorFec(0).MakeParity(MakeChunks());
  ASSERT_EQ(parity.size(), 1u);
  const std::vector<uint8_t> expected = {0x01 ^ 0xF0 ^ 0x11, 0x02 ^ 0x0F ^ 0x22,
                                         0x03 ^ 0xAA, 0x04 ^ 0x55};
  EXPECT_EQ(parity[0], std::string(expected.begin(), expected.end()));
}

TEST(XorFecTest, OneParityPerGroupAndShortLastGroup) {
  const std::vector<std::string> parity = XorFec(2).MakeParity(MakeChunks());
  ASSERT_EQ(parity.size(), 2u);
  const std::vector<uint8_t> expected = {0x01 ^ 0xF0, 0x02 ^ 0x0F, 0x03 ^ 0xAA,
                                         0x04 ^ 0x55};
  EXPECT_EQ(parity[0], std::string(expected.begin(), expected.end()));
  // A group of one chunk: its parity is the chunk itself.
  EXPECT_EQ(parity[1], std::string(kData[2].begin(), kData[2].end()));
}

TEST(XorFecTest, GroupLargerThanFrameIsOneWholeFrameGroup) {
  const XorFec fec(5);
  EXPECT_EQ(fec.GetGroupSize(kData.size()), 5u);
  const std::vector<std::string> parity = fec.MakeParity(MakeChunks());
  EXPECT_EQ(parity, XorFec(0).MakeParity(MakeChunks()));
}

TEST(XorFecTest, EmptyFrameMakesNoParity) {
  EXPECT_TRUE(XorFec(0).MakeParity({}).empty());
  EXPECT_TRUE(XorFec(2).MakeParity({}).empty());
}

TEST(XorFecTest, LongerLaterChunkExtendsParity) {
  const std::vector<std::vector<uint8_t>> data{{0x01}, {0x02, 0x03}};
  const Chunks chunks(data.begin(), data.end());
  const std::vector<std::string> parity = XorFec(0).MakeParity(chunks);
  ASSERT_EQ(parity.size(), 1u);
  const std::vector<uint8_t> expected = {0x01 ^ 0x02, 0x03};
  EXPECT_EQ(parity[0], std::string(expected.begin(), expected.end()));
}

TEST(XorFecTest, ParityRebuildsAnyOneLostChunk) {
  const std::string parity = XorFec(0).MakeParity(MakeChunks())[0];
  for (size_t lost = 0; lost < kData.size(); ++lost) {
    std::string rebuilt = parity;
    for (size_t i = 0; i < kData.size(); ++i) {
      if (i == lost) {
        continue;
      }
      for (size_t byte = 0; byte < kData[i].size(); ++byte) {
        rebuilt[byte] ^= static_cast<char>(kData[i][byte]);
      }
    }
    rebuilt.resize(kData[lost].size());
    EXPECT_EQ(rebuilt, std::string(kData[lost].begin(), kData[lost].end()))
        << "lost chunk " << lost;
  }
}

TEST(MakeFecStrategyTest, PicksStrategyFromConfig) {
  const auto xor_fec = MakeFecStrategy(config::FecScheme::kXor, 5);
  EXPECT_EQ(xor_fec->GetScheme(), drone_protocol::FEC_SCHEME_XOR);
  EXPECT_EQ(xor_fec->GetGroupSize(kData.size()), 5u);
  const auto no_fec = MakeFecStrategy(config::FecScheme::kNone, 5);
  EXPECT_EQ(no_fec->GetScheme(), drone_protocol::FEC_SCHEME_NONE);
  EXPECT_EQ(no_fec->GetGroupSize(kData.size()), 0u);
}

}
}
