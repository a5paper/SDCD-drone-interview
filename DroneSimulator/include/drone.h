#ifndef DRONE_SIMULATOR_INCLUDE_DRONE_H_
#define DRONE_SIMULATOR_INCLUDE_DRONE_H_

#include <types.h>

namespace drone_simulator::Drone
{


    struct VelocityCommand
    {
        f64 velocity_x = 0.0;
        f64 velocity_y = 0.0;
    };

    struct DroneState
    {
        f64 estimated_velocity_x = 0.0;
        f64 estimated_velocity_y = 0.0;

        u8 battery_percentage = 100U;
        u32 drone_id = 1U;
    };
}

#endif