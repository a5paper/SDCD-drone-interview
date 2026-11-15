#include "udp_socket.h"

#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace drone_simulator::network
{

    UdpSocket::UdpSocket()
        : fd_(::socket(AF_INET, SOCK_DGRAM, 0))
    {
    }

    UdpSocket::~UdpSocket()
    {
        Close_();
    }

    UdpSocket::UdpSocket(UdpSocket &&other) noexcept
        : fd_(std::exchange(other.fd_, kInvalidFd))
    {
    }

    UdpSocket &UdpSocket::operator=(UdpSocket &&other) noexcept
    {
        if (this != &other)
        {
            Close_();
            fd_ = std::exchange(other.fd_, kInvalidFd);
        }
        return *this;
    }

    b8 UdpSocket::IsOpen() const
    {
        return fd_ != kInvalidFd;
    }

    b8 UdpSocket::Bind(u16 port)
    {
        if (!IsOpen())
        {
            return false;
        }

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);

        return ::bind(fd_, reinterpret_cast<const sockaddr *>(&address),
                      sizeof(address)) == 0;
    }

    b8 UdpSocket::SetNonBlocking()
    {
        if (!IsOpen())
        {
            return false;
        }

        const i32 flags = ::fcntl(fd_, F_GETFL, 0);
        if (flags < 0)
        {
            return false;
        }

        return ::fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == 0;
    }

    ReceiveResult UdpSocket::Receive(std::span<byte_t> buffer)
    {
        if (!IsOpen())
        {
            return {ReceiveStatus::kError, 0, EBADF};
        }

        const ssize_t received = ::recv(fd_, buffer.data(), buffer.size(), 0);
        if (received < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return {ReceiveStatus::kWouldBlock, 0, 0};
            }
            return {ReceiveStatus::kError, 0, errno};
        }

        return {ReceiveStatus::kData, static_cast<u64>(received), 0};
    }

    void UdpSocket::Close_()
    {
        if (IsOpen())
        {
            ::close(fd_);
            fd_ = kInvalidFd;
        }
    }

}
