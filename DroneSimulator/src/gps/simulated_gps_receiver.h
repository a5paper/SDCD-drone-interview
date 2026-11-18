#ifndef DRONE_SIMULATOR_GPS_SIMULATED_GPS_RECEIVER_H_
#define DRONE_SIMULATOR_GPS_SIMULATED_GPS_RECEIVER_H_

#include <optional>

#include "gps/gps_receiver.h"
#include "sim/world.h"

namespace drone_simulator::gps {

class SimulatedGpsReceiver : public GpsReceiver {
 public:
  explicit SimulatedGpsReceiver(const world::World& world);

  std::optional<GpsFix> ReadFix() override;

 private:
  const world::World& world_;
};

}

#endif
