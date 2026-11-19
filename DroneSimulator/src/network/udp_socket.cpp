#include "network/udp_socket.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <cerrno>
#include <string>
#include <system_error>
#include <utility>

namespace drone_simulator::network {
namespace {

#ifdef _WIN32

constexpr WORD kWinsockVersion = MAKEWORD(2, 2);
constexpr int kBadHandleError = WSAENOTSOCK;
constexpr int kInvalidArgumentError = WSAEINVAL;

bool StartWinsock() {
  WSADATA data;
  return WSAStartup(kWinsockVersion, &data) == 0;
}

void EnsureNetworkingStarted() {
  static const bool kWinsockStarted = StartWinsock();
  (void)kWinsockStarted;
}

SOCKET ToSocket(std::uintptr_t handle) { return static_cast<SOCKET>(handle); }

void CloseNative(std::uintptr_t handle) { ::closesocket(ToSocket(handle)); }

bool SetNonBlockingNative(std::uintptr_t handle) {
  u_long enable = 1;
  return ::ioctlsocket(ToSocket(handle), FIONBIO, &enable) == 0;
}

bool IsWouldBlock(int error_code) { return error_code == WSAEWOULDBLOCK; }

void SetLastSocketError(int error_code) { ::WSASetLastError(error_code); }

#else

constexpr int kBadHandleError = EBADF;
constexpr int kInvalidArgumentError = EINVAL;

void EnsureNetworkingStarted() {}

void CloseNative(int handle) { ::close(handle); }

bool SetNonBlockingNative(int handle) {
  const int flags = ::fcntl(handle, F_GETFL, 0);
  if (flags < 0) {
    return false;
  }
  return ::fcntl(handle, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool IsWouldBlock(int error_code) {
  return error_code == EAGAIN || error_code == EWOULDBLOCK;
}

void SetLastSocketError(int error_code) { errno = error_code; }

#endif

}

int LastSocketError() {
#ifdef _WIN32
  return ::WSAGetLastError();
#else
  return errno;
#endif
}

std::string ErrorText(int error_code) {
  return std::system_category().message(error_code);
}

UdpSocket::UdpSocket() {
  EnsureNetworkingStarted();
  handle_ = ::socket(AF_INET, SOCK_DGRAM, 0);
}

UdpSocket::~UdpSocket() { Close(); }

UdpSocket::UdpSocket(UdpSocket&& other) noexcept
    : handle_(std::exchange(other.handle_, kInvalidHandle)) {}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept {
  if (this != &other) {
    Close();
    handle_ = std::exchange(other.handle_, kInvalidHandle);
  }
  return *this;
}

bool UdpSocket::IsOpen() const { return handle_ != kInvalidHandle; }

bool UdpSocket::Bind(uint16_t port) {
  if (!IsOpen()) {
    SetLastSocketError(kBadHandleError);
    return false;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);

  return ::bind(handle_, reinterpret_cast<const sockaddr*>(&address),
                sizeof(address)) == 0;
}

bool UdpSocket::SetNonBlocking() {
  if (!IsOpen()) {
    SetLastSocketError(kBadHandleError);
    return false;
  }
  return SetNonBlockingNative(handle_);
}

bool UdpSocket::Connect(std::string_view address, uint16_t port) {
  if (!IsOpen()) {
    SetLastSocketError(kBadHandleError);
    return false;
  }

  sockaddr_in destination{};
  destination.sin_family = AF_INET;
  destination.sin_port = htons(port);
  const std::string address_text(address);
  if (::inet_pton(AF_INET, address_text.c_str(), &destination.sin_addr) != 1) {
    SetLastSocketError(kInvalidArgumentError);
    return false;
  }

  return ::connect(handle_, reinterpret_cast<const sockaddr*>(&destination),
                   sizeof(destination)) == 0;
}

ReceiveResult UdpSocket::Receive(std::span<uint8_t> buffer) {
  if (!IsOpen()) {
    return {ReceiveStatus::kError, 0, kBadHandleError};
  }

#ifdef _WIN32
  const int received = ::recv(handle_, reinterpret_cast<char*>(buffer.data()),
                              static_cast<int>(buffer.size()), 0);
#else
  const ssize_t received = ::recv(handle_, buffer.data(), buffer.size(), 0);
#endif
  if (received < 0) {
    const int error_code = LastSocketError();
    if (IsWouldBlock(error_code)) {
      return {ReceiveStatus::kWouldBlock, 0, 0};
    }
    return {ReceiveStatus::kError, 0, error_code};
  }

  return {ReceiveStatus::kData, static_cast<size_t>(received), 0};
}

SendResult UdpSocket::Send(std::span<const uint8_t> data) {
  if (!IsOpen()) {
    return {SendStatus::kError, 0, kBadHandleError};
  }

#ifdef _WIN32
  const int sent = ::send(handle_, reinterpret_cast<const char*>(data.data()),
                          static_cast<int>(data.size()), 0);
#else
  const ssize_t sent = ::send(handle_, data.data(), data.size(), 0);
#endif
  if (sent < 0) {
    const int error_code = LastSocketError();
    if (IsWouldBlock(error_code)) {
      return {SendStatus::kWouldBlock, 0, 0};
    }
    return {SendStatus::kError, 0, error_code};
  }

  return {SendStatus::kSent, static_cast<size_t>(sent), 0};
}

void UdpSocket::Close() {
  if (IsOpen()) {
    CloseNative(handle_);
    handle_ = kInvalidHandle;
  }
}

}
