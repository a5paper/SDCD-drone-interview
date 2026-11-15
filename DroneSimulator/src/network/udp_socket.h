#ifndef DRONE_SIMULATOR_NETWORK_UDP_SOCKET_H_
#define DRONE_SIMULATOR_NETWORK_UDP_SOCKET_H_

#include <span>

#include "core/types.h"

namespace drone_simulator::network
{

    enum class ReceiveStatus
    {
        kData,
        kWouldBlock,
        kError, // error_code holds errno
    };

    struct ReceiveResult
    {
        ReceiveStatus status = ReceiveStatus::kError;
        u64 bytes = 0;
        i32 error_code = 0;
    };

    class UdpSocket
    {
    public:
        UdpSocket();
        ~UdpSocket();

        UdpSocket(const UdpSocket &) = delete;
        UdpSocket &operator=(const UdpSocket &) = delete;

        UdpSocket(UdpSocket &&other) noexcept;
        UdpSocket &operator=(UdpSocket &&other) noexcept;

        b8 IsOpen() const;
        [[nodiscard]] b8 Bind(u16 port);
        [[nodiscard]] b8 SetNonBlocking();

        [[nodiscard]] ReceiveResult Receive(std::span<byte_t> buffer);

    private:
        static constexpr i32 kInvalidFd = -1;

        void Close_();

        i32 fd_ = kInvalidFd;
    };

}

#endif
