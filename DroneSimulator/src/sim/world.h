#ifndef DRONE_SIMULATOR_SIM_WORLD_H_
#define DRONE_SIMULATOR_SIM_WORLD_H_

#include "core/config.h"

namespace drone_simulator::world {

struct WorldState {
  double drone_position_x = config::kWidth / 2;
  double drone_position_y = config::kHeight / 2;
  double elapsed_time = 0.0;
};

class World {
 public:
  void Update(double velocity_x, double velocity_y, double delta_time);
  const WorldState& GetState() const;

 private:
  WorldState state_;
};

}

#endif
