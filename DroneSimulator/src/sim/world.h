#ifndef DRONE_SIMULATOR_SIM_WORLD_H_
#define DRONE_SIMULATOR_SIM_WORLD_H_

#include <cstdint>
#include <vector>

#include "core/config.h"

namespace drone_simulator::world {

struct CarPool {
  std::vector<float> along;
  std::vector<float> across;
  std::vector<float> velocity;
  std::vector<uint8_t> color;
};

struct WorldState {
  double drone_position_x = config::kWorldWidthM / 2;
  double drone_position_y = config::kWorldHeightM / 2;
  double elapsed_time_s = 0.0;
  CarPool cars_along_x;
  CarPool cars_along_y;
};

class World {
 public:
  World();

  void Update(double velocity_x, double velocity_y, double delta_time);
  const WorldState& GetState() const;

 private:
  WorldState state_;
};

}

#endif
