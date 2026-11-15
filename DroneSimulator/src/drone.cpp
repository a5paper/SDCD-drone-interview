
#include "drone.h"
#include <cmath>
#include "config.h"

namespace drone_simulator::drone
{

    bool IsCommandValid_(const drone_protocol::VelocityChangeCommand &command) {
        if (!std::isfinite(command.velocity_x()) ||
            !std::isfinite(command.velocity_y()))
        {
            return false;
        }

        if (command.velocity_x() < -1.0 || command.velocity_x() > 1.0 ||
            command.velocity_y() < -1.0 || command.velocity_y() > 1.0)
        {
            return false;
        }

        return true;
    }

    void Drone::HandleCommand(const drone_protocol::VelocityChangeCommand &command)
    {

        if(!IsCommandValid_(command)) { return; }

        state_.velocity.x = command.velocity_x() * config::kMaxSpeed;
        state_.velocity.y = command.velocity_y() * config::kMaxSpeed;
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