#ifndef DRONE_SIMULATOR_INCLUDE_CONFIG_H_
#define DRONE_SIMULATOR_INCLUDE_CONFIG_H_

#include "types.h"

namespace drone_simulator::config
{
    // World
    constexpr f64 kWidth = 1000.0;
    constexpr f64 kHeight = 1000.0;

    // Simulation loop
    constexpr f64 kUpdateRateHz = 50.0;
    constexpr f64 kFPS = 1.0 / kUpdateRateHz;
    constexpr f64 kStatusPrintIntervalS = 3.0;

    // Drone
    constexpr f64 kMaxSpeed = 10.0;
    constexpr f64 kMaxBattery = 100.0;
    constexpr f64 kInitialBatteryFraction = 0.687;
    constexpr f64 kInitialBattery = kMaxBattery * kInitialBatteryFraction;
    constexpr u32 kBatteryLifetimeS = 1200;
    constexpr f64 kBatteryDrainPerS = kMaxBattery / kBatteryLifetimeS;

    // Video
    constexpr i32 kFrameWidthPx = 640;
    constexpr i32 kFrameHeightPx = 640;
    constexpr f64 kPixelsPerMeter = 10.0;
    constexpr f64 kBlockSizeM = 50.0;
    constexpr f64 kRoadWidthM = 8.0;
    // Video - Debug
    constexpr f64 kDroneSizeM = 2.0;
    constexpr b8 kShowPreview = true;
    

    // Network
    constexpr u16 kCommandPort = 5600;
    constexpr u16 kMtuSize = 1500;
}

#endif
