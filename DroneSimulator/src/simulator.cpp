#include "simulator.h"

#include "config.h"

namespace drone_simulator
{

    void Simulator::Update(f64 delta_time)
    {
        drone.Update(delta_time);

        const auto &drone_velocity = drone.GetState().velocity;
        world.Update(drone_velocity.x, drone_velocity.y, delta_time);
    }

}