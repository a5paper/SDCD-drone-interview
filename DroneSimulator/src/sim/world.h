#ifndef DRONE_SIMULATOR_SIM_WORLD_H_
#define DRONE_SIMULATOR_SIM_WORLD_H_

#include <cstdint>
#include <vector>

#include "core/config.h"

namespace drone_simulator::world {

// Cars on roads of one axis, as a structure of arrays: the update loop reads
// only `along` and `velocity`. A car's index never changes, so it is its ID.
struct CarPool {
  std::vector<float> along;     // Position along the road, m.
  std::vector<float> across;    // Lane position, m. Constant.
  std::vector<float> velocity;  // Signed speed along the road, m/s.
  std::vector<uint8_t> color;   // Index into the renderer's palette.
};

struct WorldState {
  double drone_position_x = config::kWidth / 2;
  double drone_position_y = config::kHeight / 2;
  double elapsed_time = 0.0;
  CarPool cars_along_x;  // On roads that run along X.
  CarPool cars_along_y;  // On roads that run along Y.
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
