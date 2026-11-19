#include "network/command_receiver.h"

#include <iostream>

namespace drone_simulator::network {

bool CommandReceiver::Open(uint16_t port) {
  return socket_.Bind(port) && socket_.SetNonBlocking();
}

std::optional<drone_protocol::VelocityChangeCommand> CommandReceiver::Poll() {
  std::optional<drone_protocol::VelocityChangeCommand> latest;

  ReceiveResult result;
  while ((result = socket_.Receive(buffer_)).status !=
         ReceiveStatus::kWouldBlock) {
    if (result.status == ReceiveStatus::kError) {
      std::cerr << "Receive failed: " << ErrorText(result.error_code)
                << std::endl;
      break;
    }

    drone_protocol::VelocityChangeCommand command;
    const int size = static_cast<int>(result.bytes);
    if (command.ParseFromArray(buffer_.data(), size)) {
      latest = command;
    }
  }

  return latest;
}

}
