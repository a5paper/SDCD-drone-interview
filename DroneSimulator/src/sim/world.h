#ifndef DRONE_SIMULATOR_SIM_WORLD_H_
#define DRONE_SIMULATOR_SIM_WORLD_H_

#include <cstdint>
#include <random>
#include <vector>

#include "core/config.h"

namespace drone_simulator::world {

struct Car {
  double position_x = 0.0;
  double position_y = 0.0;
  double velocity_x = 0.0;
  double velocity_y = 0.0;
  uint8_t color_index = 0;
};

struct WorldState {
  double drone_position_x = config::kWidth / 2;
  double drone_position_y = config::kHeight / 2;
  double elapsed_time = 0.0;
  std::vector<Car> cars;
};

class World {
 public:
  void Update(double velocity_x, double velocity_y, double delta_time);
  const WorldState& GetState() const;

 private:
  void UpdateCars(double delta_time);
  void SpawnCar();

  WorldState state_;
  double spawn_timer_ = 0.0;
  std::mt19937 random_{config::kRandomSeed};
};

}

#endif
