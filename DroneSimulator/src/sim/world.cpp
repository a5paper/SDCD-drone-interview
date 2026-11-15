#include "sim/world.h"

#include <algorithm>
#include <random>

#include "core/config.h"

namespace drone_simulator::world {
namespace {

// Fills both lanes of every road in `pool` with cars at random gaps. Roads are
// kBlockSizeM apart over `across_size`. `lane_sign` puts each direction in its
// right-hand lane. All cars in a lane share one speed, so gaps never shrink.
void FillLanes(CarPool& pool, double across_size, float road_length,
               double lane_sign, std::mt19937& random) {
  std::uniform_real_distribution<float> gap(config::kCarMinGapM,
                                            config::kCarMaxGapM);
  std::uniform_int_distribution<int> color(0, config::kCarColorCount - 1);
  for (double road = 0.0; road <= across_size; road += config::kBlockSizeM) {
    for (const float direction : {-1.0f, 1.0f}) {
      const auto lane = static_cast<float>(road + lane_sign * direction *
                                                      config::kLaneOffsetM);
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

// Branch-free so the compiler can vectorize it: a car that leaves one end of
// the road comes back at the other.
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
  FillLanes(state_.cars_along_x, config::kHeight, config::kWidth,
            /*lane_sign=*/1.0, random);
  FillLanes(state_.cars_along_y, config::kWidth, config::kHeight,
            /*lane_sign=*/-1.0, random);
}

void World::Update(double velocity_x, double velocity_y, double delta_time) {
  if (delta_time <= 0.0) {
    return;
  }

  state_.drone_position_x = std::clamp(
      state_.drone_position_x + velocity_x * delta_time, 0.0, config::kWidth);
  state_.drone_position_y = std::clamp(
      state_.drone_position_y + velocity_y * delta_time, 0.0, config::kHeight);
  state_.elapsed_time += delta_time;

  const auto delta = static_cast<float>(delta_time);
  MoveCars(state_.cars_along_x, config::kWidth, delta);
  MoveCars(state_.cars_along_y, config::kHeight, delta);
}

const WorldState& World::GetState() const { return state_; }

}
