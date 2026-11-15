#include "sim/world.h"

#include <algorithm>

#include "core/config.h"

namespace drone_simulator::world {

void World::Update(double velocity_x, double velocity_y, double delta_time) {
  if (delta_time <= 0.0) {
    return;
  }

  state_.drone_position_x = std::clamp(
      state_.drone_position_x + velocity_x * delta_time, 0.0, config::kWidth);
  state_.drone_position_y = std::clamp(
      state_.drone_position_y + velocity_y * delta_time, 0.0, config::kHeight);
  state_.elapsed_time += delta_time;
}

const WorldState& World::GetState() const { return state_; }

}
