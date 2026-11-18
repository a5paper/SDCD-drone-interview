#include "gps/simulated_gps_receiver.h"

#include <cmath>
#include <numbers>

#include "config/config.h"

namespace drone_simulator::gps {
namespace {

constexpr double kDegreesPerRadian = 180.0 / std::numbers::pi;

}

SimulatedGpsReceiver::SimulatedGpsReceiver(const world::World& world)
    : world_(world) {}

std::optional<GpsFix> SimulatedGpsReceiver::ReadFix() {
  const world::WorldState& state = world_.GetState();
  const double north_m = state.drone_position_x;
  const double east_m = state.drone_position_y;
  const double origin_latitude_rad =
      config::kGpsOriginLatitudeDeg / kDegreesPerRadian;
  const double east_radius_m =
      config::kEarthRadiusM * std::cos(origin_latitude_rad);
  return GpsFix{config::kGpsOriginLatitudeDeg +
                    north_m / config::kEarthRadiusM * kDegreesPerRadian,
                config::kGpsOriginLongitudeDeg +
                    east_m / east_radius_m * kDegreesPerRadian};
}

}
