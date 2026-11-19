#ifndef DRONE_SIMULATOR_NETWORK_UDP_SOCKET_H_
#define DRONE_SIMULATOR_NETWORK_UDP_SOCKET_H_

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace drone_simulator::network {

// errno on POSIX, WSAGetLastError() on Windows. Read it right after a failed
// socket call.
int LastSocketError();

// Human-readable text for a LastSocketError() value.
std::string ErrorText(int error_code);

enum class ReceiveStatus {
  kData,
  kWouldBlock,
  kError,  // error_code holds a LastSocketError() value
};

struct ReceiveResult {
  ReceiveStatus status = ReceiveStatus::kError;
  size_t bytes = 0;
  int error_code = 0;
};

enum class SendStatus {
  kSent,
  kWouldBlock,
  kError,  // error_code holds a LastSocketError() value
};

struct SendResult {
  SendStatus status = SendStatus::kError;
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
  // Sets the default destination for Send(). address is dotted IPv4.
  [[nodiscard]] bool Connect(std::string_view address, uint16_t port);

  [[nodiscard]] ReceiveResult Receive(std::span<uint8_t> buffer);
  [[nodiscard]] SendResult Send(std::span<const uint8_t> data);

 private:
#ifdef _WIN32
  using NativeHandle = std::uintptr_t;
  static constexpr NativeHandle kInvalidHandle = ~NativeHandle{0};
#else
  using NativeHandle = int;
  static constexpr NativeHandle kInvalidHandle = -1;
#endif

  void Close();

  NativeHandle handle_ = kInvalidHandle;
};

}

#endif
