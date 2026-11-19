#ifndef DRONE_SIMULATOR_SIM_DRONE_H_
#define DRONE_SIMULATOR_SIM_DRONE_H_

#include <optional>

#include "config/config.h"
#include "gps/gps_receiver.h"
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
  std::optional<gps::GpsFix> gps_fix;
};

class Drone {
 public:
  explicit Drone(gps::GpsReceiver& gps_receiver);

  void HandleCommand(const drone_protocol::VelocityChangeCommand& command,
                     double current_time_s);
  void Update(double delta_time, double current_time_s);
  void SetCommandTimeout(double command_timeout_s);
  const DroneState& GetState() const;
  Vector2 GetVelocity() const;

 private:
  void UpdateGpsFix();

  gps::GpsReceiver& gps_receiver_;
  DroneState state_;
  double command_timeout_s_ = config::kDefaultCommandTimeoutS;
  double last_command_time_s_ = 0.0;
};

}

#endif
