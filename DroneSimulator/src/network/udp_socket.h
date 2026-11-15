#ifndef DRONE_SIMULATOR_NETWORK_UDP_SOCKET_H_
#define DRONE_SIMULATOR_NETWORK_UDP_SOCKET_H_

#include <cstddef>
#include <cstdint>
#include <span>

namespace drone_simulator::network {

enum class ReceiveStatus {
  kData,
  kWouldBlock,
  kError,  // error_code holds errno
};

struct ReceiveResult {
  ReceiveStatus status = ReceiveStatus::kError;
  size_t bytes = 0;
  int error_code = 0;
};

class UdpSocket {
 public:
  UdpSocket();
  ~UdpSocket();

  UdpSocket(const UdpSocket&) = delete;
  UdpSocket& operator=(const UdpSocket&) = delete;

  UdpSocket(UdpSocket&& other) noexcept;
  UdpSocket& operator=(UdpSocket&& other) noexcept;

  bool IsOpen() const;
  [[nodiscard]] bool Bind(uint16_t port);
  [[nodiscard]] bool SetNonBlocking();

  [[nodiscard]] ReceiveResult Receive(std::span<uint8_t> buffer);

 private:
  static constexpr int kInvalidFd = -1;

  void Close();

  int fd_ = kInvalidFd;
};

}

#endif
