#ifndef DRONE_SIMULATOR_SIM_ROAD_GRID_H_
#define DRONE_SIMULATOR_SIM_ROAD_GRID_H_

#include <cmath>

#include "config/config.h"

namespace drone_simulator::world {

// Index of the block (or road) at or below `coordinate`. Roads run through
// every multiple of kBlockSizeM; block k lies between roads k and k + 1.
inline int RoadAtOrBelow(double coordinate) {
  return static_cast<int>(std::floor(coordinate / config::kBlockSizeM));
}

inline double RoadCenter(int road) { return road * config::kBlockSizeM; }

inline double LaneCenter(int road, double direction, bool along_x) {
  const double right_of_travel = along_x ? direction : -direction;
  return RoadCenter(road) + right_of_travel * config::kLaneOffsetM;
}

}

#endif
