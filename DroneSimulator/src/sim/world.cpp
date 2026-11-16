#include "sim/world.h"

#include <algorithm>
#include <random>

#include "config/config.h"
#include "sim/road_grid.h"

namespace drone_simulator::world {
namespace {

void FillLanes(CarPool& pool, double across_size, float road_length,
               bool along_x, std::mt19937& random) {
  std::uniform_real_distribution<float> gap(config::kCarMinGapM,
                                            config::kCarMaxGapM);
  std::uniform_int_distribution<int> color(0, config::kCarColorCount - 1);
  for (int road = 0; RoadCenter(road) <= across_size; ++road) {
    for (const float direction : {-1.0f, 1.0f}) {
      const auto lane =
          static_cast<float>(LaneCenter(road, direction, along_x));
      for (float rear = gap(random); rear + config::kCarLengthM <= road_length;
           rear += config::kCarLengthM + gap(random)) {
        pool.along.push_back(rear + config::kCarLengthM / 2.0f);
        pool.across.push_back(lane);
        pool.velocity.push_back(direction * config::kCarSpeedMps);
        pool.color.push_back(static_cast<uint8_t>(color(random)));
      }
    }
  }
}

// Branch-free so the compiler can vectorize it
void MoveCars(CarPool& pool, float road_length, float delta_time) {
  for (size_t i = 0; i < pool.along.size(); ++i) {
    float along = pool.along[i] + pool.velocity[i] * delta_time;
    along += road_length * ((along < 0.0f) - (along >= road_length));
    pool.along[i] = along;
  }
}

}

World::World() {
  std::mt19937 random(config::kRandomSeed);
  FillLanes(state_.cars_along_x, config::kWorldHeightM, config::kWorldWidthM,
            /*along_x=*/true, random);
  FillLanes(state_.cars_along_y, config::kWorldWidthM, config::kWorldHeightM,
            /*along_x=*/false, random);
}

void World::Update(double velocity_x, double velocity_y, double delta_time) {
  if (delta_time <= 0.0) {
    return;
  }

  state_.drone_position_x =
      std::clamp(state_.drone_position_x + velocity_x * delta_time, 0.0,
                 config::kWorldWidthM);
  state_.drone_position_y =
      std::clamp(state_.drone_position_y + velocity_y * delta_time, 0.0,
                 config::kWorldHeightM);
  state_.elapsed_time_s += delta_time;

  const auto delta = static_cast<float>(delta_time);
  MoveCars(state_.cars_along_x, config::kWorldWidthM, delta);
  MoveCars(state_.cars_along_y, config::kWorldHeightM, delta);
}

const WorldState& World::GetState() const { return state_; }

}
