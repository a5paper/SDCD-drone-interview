#ifndef DRONE_SIMULATOR_SIM_DRONE_H_
#define DRONE_SIMULATOR_SIM_DRONE_H_

#include "config/config.h"
#include "protocols/commands.pb.h"

namespace drone_simulator::drone {

struct Vector2 {
  double x = 0.0;
  double y = 0.0;
};

struct DroneState {
  Vector2 direction;
  double speed_mps = 0.0;
  double battery_percentage = config::kInitialBatteryPercent;

  drone_protocol::VelocityChangeCommand last_command;
};

class Drone {
 public:
  void HandleCommand(const drone_protocol::VelocityChangeCommand& command);
  void Update(double delta_time);
  const DroneState& GetState() const;
  Vector2 GetVelocity() const;

 private:
  DroneState state_;
};

}

#endif
