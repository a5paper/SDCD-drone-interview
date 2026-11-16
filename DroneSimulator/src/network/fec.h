#ifndef DRONE_SIMULATOR_NETWORK_FEC_H_
#define DRONE_SIMULATOR_NETWORK_FEC_H_

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "config/network_config.h"
#include "protocols/video.pb.h"

namespace drone_simulator::network {

using Chunks = std::vector<std::span<const uint8_t>>;

class FecStrategy {
 public:
  virtual ~FecStrategy() = default;

  virtual drone_protocol::FecScheme GetScheme() const = 0;
  virtual uint32_t GetGroupSize(size_t chunk_count) const = 0;
  virtual std::vector<std::string> MakeParity(const Chunks& chunks) const = 0;
};

class NoFec : public FecStrategy {
 public:
  drone_protocol::FecScheme GetScheme() const override;
  uint32_t GetGroupSize(size_t chunk_count) const override;
  std::vector<std::string> MakeParity(const Chunks& chunks) const override;
};

// One parity chunk per group of data chunks: the byte-wise XOR of the group,
// each chunk zero-padded to the longest. It rebuilds one lost chunk per group.
class XorFec : public FecStrategy {
 public:
  explicit XorFec(uint32_t group_size);

  drone_protocol::FecScheme GetScheme() const override;
  uint32_t GetGroupSize(size_t chunk_count) const override;
  std::vector<std::string> MakeParity(const Chunks& chunks) const override;

 private:
  uint32_t group_size_;
};

std::unique_ptr<FecStrategy> MakeFecStrategy(config::FecScheme scheme,
                                             uint32_t group_size);

}

#endif
