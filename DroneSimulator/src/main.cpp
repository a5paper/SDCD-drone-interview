#include <iostream>
#include <array>
#include <chrono>
#include <thread>
#include <iomanip>
#include <format>
#include <cstring>

#include "simulator.h"
#include "udp_socket.h"
#include "types.h"
#include "config.h"

int main()
{
    using namespace std::chrono;
    using Clock = steady_clock;
    using namespace drone_simulator;

    const auto frame_duration = duration<f64>(
        config::kFPS);

    auto previous_time = Clock::now();

    // Debug
    std::cout << std::fixed << std::setprecision(2);

    f64 seconds_timer = 0.0;
    f64 seconds_duration = 3.0;
    auto start_time = previous_time;

    // Set up network
    network::UdpSocket sock;
    if (!sock.Bind(config::kCommandPort) || !sock.SetNonBlocking())
    {
        const int err = errno;
        std::cerr << std::strerror(err) << std::endl;
        return err;
    }

    std::cout << "Listening for commands on UDP "
              << config::kCommandPort << std::endl;

    // Start Main Loop
    Simulator simulator = {};
    std::cout << "Drone Simulator starting..." << std::endl;

    auto rx_buffer = std::array<u8, config::kMtuSize>();

    while (true)
    {
        const auto current_time = Clock::now();

        const f64 delta_time =
            duration<f64>(
                current_time - previous_time)
                .count();

        previous_time = current_time;

        // Periodic status printout
        seconds_timer += delta_time;
        if (seconds_timer >= seconds_duration)
        {
            seconds_timer = 0.0;
            std::cout << "Position X: "
                      << simulator.world.state.drone_position_x << " m\n";

            std::cout << "Position Y: "
                      << simulator.world.state.drone_position_y << " m\n";

            std::cout << "Battery: "
                      << simulator.drone.GetState().battery_percentage << " %\n";

            std::cout << "Time On: "
                      << std::format("{:%T}", current_time - start_time) << "\n\n";
        }

        // Rx Loop
        network::ReceiveResult res;
        while (network::ReceiveStatus::kWouldBlock != (res = sock.Receive(rx_buffer)).status)
        {
            if (res.status == network::ReceiveStatus::kError)
            {
                std::cerr << "Receive failed: " << std::strerror(res.error_code) << std::endl;
                break;
            }

            drone_protocol::VelocityChangeCommand command;
            if (command.ParseFromArray(rx_buffer.data(), static_cast<int>(res.bytes)))
            {
                simulator.drone.HandleCommand(command);
            }
        }

        simulator.Update(delta_time);

        std::this_thread::sleep_until(
            current_time + duration_cast<Clock::duration>(
                               frame_duration));
    }

    return 0;
}