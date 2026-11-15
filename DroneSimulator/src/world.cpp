#include "world.h"

#include "config.h"

namespace drone_simulator::world
{

    void World::Update(f64 velocity_x, f64 velocity_y, f64 delta_time)
    {
        if (delta_time <= 0.0)
        {
            return;
        }

        state.drone_position_x += velocity_x * delta_time;
        state.drone_position_y += velocity_y * delta_time;

        // AABB boundry limit
        if (state.drone_position_x < 0.0)
        {
            state.drone_position_x = 0.0;
        }

        if (state.drone_position_x > config::kWidth)
        {
            state.drone_position_x = config::kWidth;
        }

        if (state.drone_position_y < 0.0)
        {
            state.drone_position_y = 0.0;
        }

        if (state.drone_position_y > config::kHeight)
        {
            state.drone_position_y = config::kHeight;
        }

        state.elapsed_time += delta_time;
    }

}
