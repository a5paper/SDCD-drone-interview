#ifndef DRONE_SIMULATOR_SIM_SIMULATOR_H_
#define DRONE_SIMULATOR_SIM_SIMULATOR_H_

#include "protocols/commands.pb.h"
#include "sim/drone.h"
#include "sim/world.h"

namespace drone_simulator {

class Simulator {
 public:
  void HandleCommand(const drone_protocol::VelocityChangeCommand& command);
  void Update(double delta_time);

  const drone::Drone& GetDrone() const;
  const world::World& GetWorld() const;

 private:
  drone::Drone drone_;
  world::World world_;
};

}

#endif
