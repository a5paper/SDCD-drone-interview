#ifndef DRONE_SIMULATOR_INCLUDE_UDP_SOCKET_H_
#define DRONE_SIMULATOR_INCLUDE_UDP_SOCKET_H_

#include <span>

#include "types.h"

namespace drone_simulator::network
{

    enum class ReceiveStatus
    {
        kData,       // A datagram was read into the buffer.
        kWouldBlock, // No datagram pending (non-blocking socket).
        kError,      // Socket error; see error_code (errno).
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
        b8 Bind(u16 port);
        b8 SetNonBlocking();

        ReceiveResult Receive(std::span<byte_t> buffer);

    private:
        void Close_();

        i32 fd_ = -1;
    };

}

#endif
