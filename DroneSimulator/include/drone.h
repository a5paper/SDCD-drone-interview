#ifndef DRONE_SIMULATOR_INCLUDE_DRONE_H_
#define DRONE_SIMULATOR_INCLUDE_DRONE_H_

#include <types.h>

namespace drone_simulator::drone
{

    struct VelocityCommand
    {
        f64 x = 0.0;
        f64 y = 0.0;
    };

    struct DroneVelocity
    {
        f64 x = 0.0;
        f64 y = 0.0;
    };

    struct DroneState
    {
        DroneVelocity velocity = DroneVelocity{};
        f64 battery_percentage = 100U;
        u32 drone_id = 1U;
    };

    class Drone
    {
    public:
        Drone() = default;
        void HandleCommand(const VelocityCommand &command);
        void Update(f64 delta_time);
        const DroneState &GetState() const;

    private:
        DroneState state_{};
    };

}

#endif