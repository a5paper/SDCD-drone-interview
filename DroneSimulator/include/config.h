
#ifndef DRONE_SIMULATOR_INCLUDE_CONFIG_H_
#define DRONE_SIMULATOR_INCLUDE_CONFIG_H_

#include <types.h>

namespace drone_simulator::config
{
    constexpr f64 kWidth = 1000.0;
    constexpr f64 kHeight = 1000.0;

    constexpr f64 kUpdateRateHz = 50;
    constexpr f64 kTimeStep = 1.0 / kUpdateRateHz;
    constexpr f64 kMaxVelocity = 1.0;
    constexpr f64 kInitialBattery = 100.0;
}

#endif
