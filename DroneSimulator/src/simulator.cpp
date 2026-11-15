#include "simulator.h"

#include "config.h"

namespace drone_simulator
{

    void Simulator::Update(f64 delta_time)
    {
        drone.Update(delta_time);

        auto& droneVelocity = drone.GetState().velocity;
        world.Update(droneVelocity.x, droneVelocity.y, delta_time);
    }

}