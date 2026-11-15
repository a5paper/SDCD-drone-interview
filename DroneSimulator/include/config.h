
#ifndef DRONE_SIMULATOR_INCLUDE_CONFIG_H_
#define DRONE_SIMULATOR_INCLUDE_CONFIG_H_

#include <types.h>

namespace drone_simulator::config
{
    constexpr f64 kWidth = 1000.0;
    constexpr f64 kHeight = 1000.0;



    constexpr f64 kUpdateRateHz = 50;
    constexpr f64 kFPS = 1.0 / kUpdateRateHz;
    constexpr f64 kMaxSpeed = 10.0;

    constexpr f64 kMaxBattery = 100.0;
    constexpr f64 kInitialBattery =  kMaxBattery*0.6;
    constexpr uint32_t kBatteryLifetimeS = 120;
    constexpr f64 kBatteryDrainPerS = kMaxBattery/kBatteryLifetimeS;
}

#endif
