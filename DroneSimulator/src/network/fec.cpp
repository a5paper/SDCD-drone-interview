#include "network/fec.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace drone_simulator::network {

drone_protocol::FecScheme NoFec::GetScheme() const {
  return drone_protocol::FEC_SCHEME_NONE;
}

uint32_t NoFec::GetGroupSize(size_t) const { return 0; }

std::vector<std::string> NoFec::MakeParity(const Chunks&) const { return {}; }

XorFec::XorFec(uint32_t group_size) : group_size_(group_size) {}

drone_protocol::FecScheme XorFec::GetScheme() const {
  return drone_protocol::FEC_SCHEME_XOR;
}

uint32_t XorFec::GetGroupSize(size_t chunk_count) const {
  return group_size_ == 0 ? static_cast<uint32_t>(chunk_count) : group_size_;
}

std::vector<std::string> XorFec::MakeParity(const Chunks& chunks) const {
  std::vector<std::string> parity;
  const size_t group_size = GetGroupSize(chunks.size());
  if (group_size == 0) {
    return parity;
  }

  for (size_t first = 0; first < chunks.size(); first += group_size) {
    std::string& group_parity = parity.emplace_back();
    const size_t end = std::min(first + group_size, chunks.size());
    for (size_t i = first; i < end; ++i) {
      if (group_parity.size() < chunks[i].size()) {
        group_parity.resize(chunks[i].size(), '\0');
      }
      for (size_t byte = 0; byte < chunks[i].size(); ++byte) {
        group_parity[byte] ^= static_cast<char>(chunks[i][byte]);
      }
    }
  }
  return parity;
}

std::unique_ptr<FecStrategy> MakeFecStrategy(config::FecScheme scheme,
                                             uint32_t group_size) {
  if (scheme == config::FecScheme::kXor) {
    return std::make_unique<XorFec>(group_size);
  }
  return std::make_unique<NoFec>();
}

}
