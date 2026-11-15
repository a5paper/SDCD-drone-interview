#ifndef DRONE_SIMULATOR_CORE_CONFIG_H_
#define DRONE_SIMULATOR_CORE_CONFIG_H_

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace drone_simulator::config {

// World
inline constexpr double kWidth = 1000.0;
inline constexpr double kHeight = 1000.0;

// Simulation loop
inline constexpr double kUpdateRateHz = 50.0;
inline constexpr double kTickPeriodS = 1.0 / kUpdateRateHz;
inline constexpr double kStatusPrintIntervalS = 3.0;
inline constexpr int kStatusPrintPrecision = 2;
inline constexpr double kBytesPerKiB = 1024.0;

// Drone
inline constexpr double kMaxCommandVelocity = 1;
inline constexpr double kMaxSpeed = 10.0;
inline constexpr double kMaxBattery = 100.0;
inline constexpr double kInitialBatteryFraction = 0.687;
inline constexpr double kInitialBattery = kMaxBattery * kInitialBatteryFraction;
inline constexpr double kBatteryLifetimeS = 1200.0;
inline constexpr double kBatteryDrainPerS = kMaxBattery / kBatteryLifetimeS;

// Video
inline constexpr int kFrameWidthPx = 640;
inline constexpr int kFrameHeightPx = 640;
inline constexpr double kPixelsPerMeter = 10.0;
inline constexpr double kBlockSizeM = 50.0;
inline constexpr double kRoadWidthM = 8.0;
// Video - Stream
inline constexpr int kTicksPerFrame = 2;
inline constexpr double kVideoFps = kUpdateRateHz / kTicksPerFrame;
inline constexpr int kJpegQuality = 70;
// Video - Debug
inline constexpr double kDroneSizeM = 2.0;
inline constexpr int kDroneCenterRadiusPx = 3;
inline constexpr bool kShowPreview = true;
inline constexpr int kPreviewWaitMs = 1;

// Network
inline constexpr uint16_t kCommandPort = 5600;
inline constexpr std::string_view kGcsAddress = "127.0.0.1";
inline constexpr uint16_t kVideoPort = 5602;
inline constexpr size_t kMtuSize = 1500;
inline constexpr size_t kMaxChunkPayload = 1400;

}

#endif
