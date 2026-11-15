#ifndef DRONE_SIMULATOR_INCLUDE_WORLD_H_
#define DRONE_SIMULATOR_INCLUDE_WORLD_H_

#include "types.h"

namespace drone_simulator::world
{

    constexpr f64 kWidth = 1000.0;
    constexpr f64 kHeight = 1000.0;
    
    struct WorldState
    {
        f64 drone_position_x = 0.0;
        f64 drone_position_y = 0.0;

        f64 elapsed_time = 0.0;
    };

}

#endif