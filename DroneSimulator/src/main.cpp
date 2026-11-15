#include <array>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iomanip>
#include <iostream>
#include <thread>

#include <opencv2/highgui.hpp>

#include "config.h"
#include "renderer.h"
#include "simulator.h"
#include "types.h"
#include "udp_socket.h"

int main()
{
    using namespace std::chrono;
    using Clock = steady_clock;
    using namespace drone_simulator;

    const auto tick_duration = duration<f64>(config::kTickPeriodS);

    auto previous_time = Clock::now();

    // Debug
    std::cout << std::fixed << std::setprecision(config::kStatusPrintPrecision);

    f64 status_print_timer = 0.0;
    const auto start_time = previous_time;

    // Set up network
    network::UdpSocket sock;
    if (!sock.Bind(config::kCommandPort) || !sock.SetNonBlocking())
    {
        const i32 err = errno;
        std::cerr << std::strerror(err) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Listening for commands on UDP " << config::kCommandPort << std::endl;

    auto rx_buffer = std::array<byte_t, config::kMtuSize>();

    // Simulation and rendering
    Simulator simulator = {};
    render::Renderer renderer;
    u64 tick = 0;

    // Main loop
    std::cout << "Drone Simulator starting..." << std::endl;
    while (true)
    {
        const auto current_time = Clock::now();

        const f64 delta_time = duration<f64>(current_time - previous_time).count();

        previous_time = current_time;

        // Periodic status printout
        status_print_timer += delta_time;
        if (status_print_timer >= config::kStatusPrintIntervalS)
        {
            status_print_timer = 0.0;
            std::cout << "Position X: " << simulator.world.state.drone_position_x << " m\n";
            std::cout << "Position Y: " << simulator.world.state.drone_position_y << " m\n";
            std::cout << "Battery: " << simulator.drone.GetState().battery_percentage << " %\n";
            std::cout << "Time On: " << std::format("{:%T}", current_time - start_time) << "\n\n";
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

        if (tick++ % config::kTicksPerFrame == 0)
        {
            const cv::Mat &frame = renderer.Render(simulator.world.state);
            if (config::kShowPreview)
            {
                cv::imshow("Drone camera", frame);
                cv::waitKey(config::kPreviewWaitMs);
            }
        }

        std::this_thread::sleep_until(current_time +
                                      duration_cast<Clock::duration>(tick_duration));
    }

    return 0;
}