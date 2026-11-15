#include "sim/world.h"

#include <algorithm>
#include <cmath>

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
  UpdateCars(delta_time);
}

const WorldState& World::GetState() const { return state_; }

void World::UpdateCars(double delta_time) {
  for (Car& car : state_.cars) {
    car.position_x += car.velocity_x * delta_time;
    car.position_y += car.velocity_y * delta_time;
  }
  std::erase_if(state_.cars, [this](const Car& car) {
    return std::hypot(car.position_x - state_.drone_position_x,
                      car.position_y - state_.drone_position_y) >
           config::kCarDespawnDistanceM;
  });

  spawn_timer_ += delta_time;
  if (spawn_timer_ >= config::kCarSpawnIntervalS) {
    spawn_timer_ -= config::kCarSpawnIntervalS;
    SpawnCar();
  }
}

// Starts a car on a road near the drone, outside the view, driving towards
// the drone in the right-hand lane.
void World::SpawnCar() {
  const bool along_x = random_() % 2 == 0;
  const double direction = random_() % 2 == 0 ? 1.0 : -1.0;
  std::uniform_int_distribution<int> road_offset(-config::kCarSpawnRoadRange,
                                                 config::kCarSpawnRoadRange);
  std::uniform_int_distribution<int> color(0, config::kCarColorCount - 1);

  const double along =
      along_x ? state_.drone_position_x : state_.drone_position_y;
  const double across =
      along_x ? state_.drone_position_y : state_.drone_position_x;
  const double road =
      (std::round(across / config::kBlockSizeM) + road_offset(random_)) *
      config::kBlockSizeM;
  const double start = along - direction * config::kCarSpawnDistanceM;
  const double speed = direction * config::kCarSpeedMps;
  const double lane = direction * config::kLaneOffsetM;
  const auto car_color = static_cast<uint8_t>(color(random_));

  if (along_x) {
    state_.cars.push_back({start, road + lane, speed, 0.0, car_color});
  } else {
    state_.cars.push_back({road - lane, start, 0.0, speed, car_color});
  }
}

}
