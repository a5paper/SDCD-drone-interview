#ifndef DRONE_SIMULATOR_SIM_SIMULATOR_H_
#define DRONE_SIMULATOR_SIM_SIMULATOR_H_

#include "sim/drone.h"
#include "sim/world.h"

namespace drone_simulator
{

    class Simulator
    {
    public:
        Simulator() = default;

        void Update(f64 delta_time);

        drone::Drone drone{};
        world::World world{};
    };

}

#endif
