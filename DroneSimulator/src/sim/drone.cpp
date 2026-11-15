#include "sim/drone.h"

#include <cmath>

#include "core/config.h"

namespace drone_simulator::drone {
namespace {

bool IsCommandValid(const drone_protocol::VelocityChangeCommand& command) {
  if (!std::isfinite(command.velocity_x()) ||
      !std::isfinite(command.velocity_y())) {
    return false;
  }

  return std::abs(command.velocity_x()) <= config::kMaxCommandVelocity &&
         std::abs(command.velocity_y()) <= config::kMaxCommandVelocity;
}

}

void Drone::HandleCommand(
    const drone_protocol::VelocityChangeCommand& command) {
  if (!IsCommandValid(command)) {
    return;
  }

  state_.velocity.x = command.velocity_x() * config::kMaxSpeed;
  state_.velocity.y = command.velocity_y() * config::kMaxSpeed;
}

void Drone::Update(double delta_time) {
  if (!std::isfinite(delta_time) || delta_time <= 0.0) {
    return;
  }

  // Handle Battery
  state_.battery_percentage -= delta_time * config::kBatteryDrainPerS;

  if (state_.battery_percentage <= 0.0) {
    state_.battery_percentage = 0.0;
    state_.velocity = {};
  }
}

const DroneState& Drone::GetState() const { return state_; }

}
