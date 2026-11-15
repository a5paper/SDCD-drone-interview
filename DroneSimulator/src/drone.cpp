
#include "drone.h"
#include <cmath>
#include "config.h"

namespace drone_simulator::drone
{

    void Drone::HandleCommand(const VelocityCommand &command)
    {
        if (state_.battery_percentage == 0U)
        {
            return;
        }

        if (!std::isfinite(command.x) ||
            !std::isfinite(command.y))
        {
            return;
        }

        // Todo(seani) move to validate function
        if (command.x < -1.0 || command.x > 1.0 ||
            command.y < -1.0 || command.y > 1.0)
        {
            return;
        }

        state_.velocity.x = command.x * config::kMaxSpeed;
        state_.velocity.y = command.y * config::kMaxSpeed;
    }

    void Drone::Update(f64 delta_time)
    {
        if (!std::isfinite(delta_time) || delta_time <= 0.0)
        {
            return;
        }

        // Handle Battery
        state_.battery_percentage -= delta_time * config::kBatteryDrainPerS;

        if (state_.battery_percentage <= 0.0)
        {
            // Clamps to avoid negative numbers
            state_.battery_percentage = 0.0;

            state_.velocity.x = 0.0;
            state_.velocity.y = 0.0;
        }
    }

    const DroneState &Drone::GetState() const
    {
        return state_;
    }

}