#ifndef DRONE_SIMULATOR_SIM_DRONE_H_
#define DRONE_SIMULATOR_SIM_DRONE_H_

#include <cstdint>

#include "core/config.h"
#include "protocols/commands.pb.h"

namespace drone_simulator::drone {

struct Vector2 {
  double x = 0.0;
  double y = 0.0;
};

struct DroneState {
  Vector2 direction;
  double speed = 0.0;
  double battery_percentage = config::kInitialBattery;
  uint32_t drone_id = 1;

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
