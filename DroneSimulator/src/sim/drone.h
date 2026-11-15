#ifndef DRONE_SIMULATOR_SIM_DRONE_H_
#define DRONE_SIMULATOR_SIM_DRONE_H_

#include "core/config.h"
#include "protocols/commands.pb.h"

namespace drone_simulator::drone {

struct Vector2 {
  double x = 0.0;
  double y = 0.0;
};

struct DroneState {
  Vector2 direction;   // Unit length, or zero when stopped.
  double speed = 0.0;  // m/s, in [0, kMaxSpeed].
  double battery_percentage = config::kInitialBattery;
  int drone_id = 1;
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
