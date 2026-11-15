#ifndef DRONE_SIMULATOR_INCLUDE_WORLD_H_
#define DRONE_SIMULATOR_INCLUDE_WORLD_H_

#include "types.h"

namespace drone_simulator::world
{

    struct WorldState
    {
        f64 drone_position_x = 0.0;
        f64 drone_position_y = 0.0;

        f64 elapsed_time = 0.0;
    };

    class World
    {
    public:
        World() = default;
        WorldState state{};

        void Update(f64 velocity_x, f64 velocity_y, f64 delta_time);

    };

}

#endif