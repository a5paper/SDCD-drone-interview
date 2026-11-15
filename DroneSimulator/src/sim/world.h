#ifndef DRONE_SIMULATOR_SIM_WORLD_H_
#define DRONE_SIMULATOR_SIM_WORLD_H_

#include "core/config.h"
#include "core/types.h"

namespace drone_simulator::world
{

    struct WorldState
    {
        f64 drone_position_x = config::kWidth / 2;
        f64 drone_position_y = config::kHeight / 2;

        f64 elapsed_time = 0.0;
    };

    class World
    {
    public:
        World() = default;
        WorldState state{};

        void Update(f64 velocity_x, f64 velocity_y, f64 delta_time);
    };

} // namespace drone_simulator::world

#endif