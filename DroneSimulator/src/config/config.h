#ifndef DRONE_SIMULATOR_CONFIG_CONFIG_H_
#define DRONE_SIMULATOR_CONFIG_CONFIG_H_

#include <cstddef>
#include <cstdint>

namespace drone_simulator::config {

// World
inline constexpr double kWorldWidthM = 1000.0;
inline constexpr double kWorldHeightM = 1000.0;
inline constexpr uint32_t kRandomSeed = 42;
inline constexpr double kGpsOriginLatitudeDeg = 31.2518;
inline constexpr double kGpsOriginLongitudeDeg = 34.7913;
inline constexpr double kEarthRadiusM = 6371000.0;
// World - Roads
inline constexpr double kBlockSizeM = 50.0;
inline constexpr double kRoadWidthM = 8.0;
inline constexpr double kLaneOffsetM = kRoadWidthM / 4.0;
// World - Buildings (placed per block, inside the roads)
inline constexpr uint8_t kBuildingsPerBlock = 2;
inline constexpr double kBuildingMarginM = 3.0;
inline constexpr double kBuildingMinSizeM = 8.0;
inline constexpr double kBuildingMaxSizeM = 18.0;
// World - Cars (fill every lane at start, wrap around at the world edge)
inline constexpr float kCarLengthM = 4.5f;
inline constexpr float kCarWidthM = 2.0f;
inline constexpr float kCarSpeedMps = 8.0f;
inline constexpr float kCarMinGapM = 3.0f;
inline constexpr float kCarMaxGapM = 25.0f;
inline constexpr uint8_t kCarColorCount = 4;

// Simulation loop
inline constexpr double kUpdateRateHz = 60.0;
inline constexpr double kTickPeriodS = 1.0 / kUpdateRateHz;
inline constexpr double kStatusPrintIntervalS = 3.0;
inline constexpr int kStatusPrintPrecision = 2;
inline constexpr double kBytesPerKiB = 1024.0;

// Drone
inline constexpr double kMaxCommandVelocity = 1.0;
inline constexpr double kMaxSpeedMps = 10.0;
inline constexpr double kMaxBatteryPercent = 100.0;
inline constexpr double kInitialBatteryPercent = 68.7;
inline constexpr double kBatteryLifeS = 1200.0;
inline constexpr double kBatteryDrainPerS = kMaxBatteryPercent / kBatteryLifeS;
inline constexpr double kDefaultCommandTimeoutS = 1.5;

// Video
inline constexpr int kFrameWidthPx = 640;
inline constexpr int kFrameHeightPx = 640;
inline constexpr double kPixelsPerMeter = 10.0;
// Video - Stream
inline constexpr uint8_t kTicksPerFrame = 2;
inline constexpr double kVideoFps = kUpdateRateHz / kTicksPerFrame;
inline constexpr int kJpegQuality = 70;
// Video - Debug
inline constexpr int kPreviewWaitMs = 1;

// Telemetry
inline constexpr double kTelemetryRateHz = 10.0;
inline constexpr uint8_t kTicksPerTelemetry = kUpdateRateHz / kTelemetryRateHz;
static_assert(kTicksPerTelemetry * kTelemetryRateHz == kUpdateRateHz);

// Network
inline constexpr size_t kMtuBytes = 1500;
inline constexpr size_t kMaxChunkPayloadBytes = 1400;

}

#endif
