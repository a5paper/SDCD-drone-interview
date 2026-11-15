#ifndef DRONE_SIMULATOR_INCLUDE_CONFIG_H_
#define DRONE_SIMULATOR_INCLUDE_CONFIG_H_

#include <string_view>

#include "types.h"

namespace drone_simulator::config
{
    // World
    constexpr f64 kWidth = 1000.0;
    constexpr f64 kHeight = 1000.0;

    // Simulation loop
    constexpr f64 kUpdateRateHz = 50.0;
    constexpr f64 kTickPeriodS = 1.0 / kUpdateRateHz;
    constexpr f64 kStatusPrintIntervalS = 3.0;
    constexpr i32 kStatusPrintPrecision = 2;

    // Drone
    constexpr f64 kMaxCommandVelocity = 1.0;
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
    // Video - Stream
    constexpr f64 kVideoFps = 25.0;
    constexpr u32 kTicksPerFrame = static_cast<u32>(kUpdateRateHz / kVideoFps);
    static_assert(kTicksPerFrame * kVideoFps == kUpdateRateHz,
                  "kUpdateRateHz must be a whole multiple of kVideoFps");
    // Video - Debug
    constexpr f64 kDroneSizeM = 2.0;
    constexpr i32 kDroneCenterRadiusPx = 3;
    constexpr b8 kShowPreview = true;
    constexpr i32 kPreviewWaitMs = 1;

    // Network
    constexpr u16 kCommandPort = 5600;
    constexpr std::string_view kGcsAddress = "127.0.0.1";
    constexpr u16 kVideoPort = 5602;
    constexpr u16 kMtuSize = 1500;
}

#endif
