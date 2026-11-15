#ifndef DRONE_SIMULATOR_SIM_DRONE_H_
#define DRONE_SIMULATOR_SIM_DRONE_H_

#include "core/config.h"
#include "protocols/commands.pb.h"

namespace drone_simulator::drone {

struct DroneVelocity {
  double x = 0.0;
  double y = 0.0;
};

struct DroneState {
  DroneVelocity velocity;
  double battery_percentage = config::kInitialBattery;
  int drone_id = 1;
};

class Drone {
 public:
  void HandleCommand(const drone_protocol::VelocityChangeCommand& command);
  void Update(double delta_time);
  const DroneState& GetState() const;

 private:
  DroneState state_;
};

}

#endif
