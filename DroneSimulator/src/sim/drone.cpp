#include "sim/drone.h"

#include <algorithm>
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

  // The command's length is the fraction of max speed, capped at 1 so
  // diagonals are not faster than straight lines.
  const double length = std::hypot(command.velocity_x(), command.velocity_y());

  if (length == 0.0) {
    state_.direction = {};
    state_.speed = 0.0;
    return;
  }

  state_.direction = {command.velocity_x() / length,
                      command.velocity_y() / length};
  state_.speed =
      std::min(length, config::kMaxCommandVelocity) * config::kMaxSpeed;
}

void Drone::Update(double delta_time) {
  if (!std::isfinite(delta_time) || delta_time <= 0.0) {
    return;
  }

  // Handle Battery
  state_.battery_percentage -= delta_time * config::kBatteryDrainPerS;

  if (state_.battery_percentage <= 0.0) {
    state_.battery_percentage = 0.0;
    state_.speed = 0.0;
  }
}

const DroneState& Drone::GetState() const { return state_; }

Vector2 Drone::GetVelocity() const {
  return {state_.direction.x * state_.speed, state_.direction.y * state_.speed};
}

}
