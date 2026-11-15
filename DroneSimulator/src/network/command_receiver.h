#ifndef DRONE_SIMULATOR_NETWORK_COMMAND_RECEIVER_H_
#define DRONE_SIMULATOR_NETWORK_COMMAND_RECEIVER_H_

#include <array>
#include <optional>

#include <commands.pb.h>

#include "core/config.h"
#include "core/types.h"
#include "network/udp_socket.h"

namespace drone_simulator::network
{

    class CommandReceiver
    {
    public:
        [[nodiscard]] b8 Open(u16 port);

        // Drains every pending datagram and returns the last one that parsed.
        std::optional<drone_protocol::VelocityChangeCommand> Poll();

    private:
        UdpSocket socket_;
        std::array<byte_t, config::kMtuSize> buffer_{};
    };

}

#endif
