#include <iostream>
#include <chrono>
#include <thread>
#include <simulator.h>

#include "types.h"

int main()
{
    using Clock = std::chrono::steady_clock;
    auto previous_time = Clock::now();

    // Debug
    f64 zigzag_timer = 0.0;
    f64 zigzag_timing = 3.0;

    drone_simulator::drone::VelocityCommand command1 = {1, 1};
    drone_simulator::drone::VelocityCommand command2 = {-1, -1};
    bool c1 = false;

    drone_simulator::Simulator simulator = {};

    std::cout << "Drone Simulator starting..." << std::endl;

    while (true)
    {
        const auto current_time = Clock::now();

        const f64 delta_time =
            std::chrono::duration<f64>(
                current_time - previous_time)
                .count();

        previous_time = current_time;

        // Debug movement
        zigzag_timer += delta_time;
        if (zigzag_timer >= zigzag_timing)
        {
            zigzag_timer = 0.0;
            c1 = !c1;

            simulator.drone.HandleCommand(c1 ? command1 : command2);
        }

        simulator.Update(delta_time);

        std::cout << "Position X: "
                  << simulator.world.state.drone_position_x << " m\n";

        std::cout << "Position Y: "
                  << simulator.world.state.drone_position_y << " m\n";
    }

    return 0;
}