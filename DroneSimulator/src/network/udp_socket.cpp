#include "network/udp_socket.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <string>
#include <utility>

namespace drone_simulator::network {

UdpSocket::UdpSocket() : fd_(::socket(AF_INET, SOCK_DGRAM, 0)) {}

UdpSocket::~UdpSocket() { Close(); }

UdpSocket::UdpSocket(UdpSocket&& other) noexcept
    : fd_(std::exchange(other.fd_, kInvalidFd)) {}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept {
  if (this != &other) {
    Close();
    fd_ = std::exchange(other.fd_, kInvalidFd);
  }
  return *this;
}

bool UdpSocket::IsOpen() const { return fd_ != kInvalidFd; }

bool UdpSocket::Bind(uint16_t port) {
  if (!IsOpen()) {
    return false;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);

  return ::bind(fd_, reinterpret_cast<const sockaddr*>(&address),
                sizeof(address)) == 0;
}

bool UdpSocket::SetNonBlocking() {
  if (!IsOpen()) {
    return false;
  }

  const int flags = ::fcntl(fd_, F_GETFL, 0);
  if (flags < 0) {
    return false;
  }

  return ::fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool UdpSocket::Connect(std::string_view address, uint16_t port) {
  if (!IsOpen()) {
    return false;
  }

  sockaddr_in destination{};
  destination.sin_family = AF_INET;
  destination.sin_port = htons(port);
  if (::inet_pton(AF_INET, std::string(address).c_str(),
                  &destination.sin_addr) != 1) {
    errno = EINVAL;
    return false;
  }

  return ::connect(fd_, reinterpret_cast<const sockaddr*>(&destination),
                   sizeof(destination)) == 0;
}

ReceiveResult UdpSocket::Receive(std::span<uint8_t> buffer) {
  if (!IsOpen()) {
    return {ReceiveStatus::kError, 0, EBADF};
  }

  const ssize_t received = ::recv(fd_, buffer.data(), buffer.size(), 0);
  if (received < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return {ReceiveStatus::kWouldBlock, 0, 0};
    }
    return {ReceiveStatus::kError, 0, errno};
  }

  return {ReceiveStatus::kData, static_cast<size_t>(received), 0};
}

SendResult UdpSocket::Send(std::span<const uint8_t> data) {
  if (!IsOpen()) {
    return {SendStatus::kError, 0, EBADF};
  }

  const ssize_t sent = ::send(fd_, data.data(), data.size(), 0);
  if (sent < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return {SendStatus::kWouldBlock, 0, 0};
    }
    return {SendStatus::kError, 0, errno};
  }

  return {SendStatus::kSent, static_cast<size_t>(sent), 0};
}

void UdpSocket::Close() {
  if (IsOpen()) {
    ::close(fd_);
    fd_ = kInvalidFd;
  }
}

}
