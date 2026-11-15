#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <format>

#include <simulator.h>
#include "types.h"
#include "config.h"

int main()
{
    using namespace std::chrono;
    using Clock = steady_clock;

    const auto frame_duration = duration<f64>(
        drone_simulator::config::kFPS);

    auto previous_time = Clock::now();

    // Debug
    std::cout << std::fixed << std::setprecision(2);

    f64 zigzag_timer = 0.0;
    f64 zigzag_timing = 3.0;
    auto start_time = previous_time;

    drone_protocol::VelocityChangeCommand command1;
    command1.set_velocity_x(1);
    command1.set_velocity_y(-0.5);

    drone_protocol::VelocityChangeCommand command2;
    command2.set_velocity_x(1);
    command2.set_velocity_y(0.5);

    bool c1 = false;

    // Start Main Loop
    drone_simulator::Simulator simulator = {};

    std::cout << "Drone Simulator starting..." << std::endl;

    while (true)
    {
        const auto current_time = Clock::now();

        const f64 delta_time =
            duration<f64>(
                current_time - previous_time)
                .count();

        previous_time = current_time;

        // Debug movement
        zigzag_timer += delta_time;
        if (zigzag_timer >= zigzag_timing)
        {
            zigzag_timer = 0.0;
            c1 = !c1;
            std::cout << "Position X: "
                      << simulator.world.state.drone_position_x << " m\n";

            std::cout << "Position Y: "
                      << simulator.world.state.drone_position_y << " m\n";

            std::cout << "Battery: "
                      << simulator.drone.GetState().battery_percentage << " %\n";

            std::cout << "Time On: "
                      << std::format("{:%T}", current_time-start_time) << "\n\n";
        }

        simulator.drone.HandleCommand(c1 ? command1 : command2);
        simulator.Update(delta_time);

        std::this_thread::sleep_until(
            current_time + duration_cast<Clock::duration>(
                               frame_duration));
    }

    return 0;
}