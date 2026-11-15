#include "simulator.h"

#include "config.h"

namespace drone_simulator
{

    void Simulator::Update(f64 delta_time)
    {
        world.state.drone_position_x +=
            drone.GetState().velocity.x * delta_time;

        world.state.drone_position_y +=
            drone.GetState().velocity.y * delta_time;

        world.state.elapsed_time += delta_time;
    }

}