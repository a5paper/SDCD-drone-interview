#ifndef DRONE_SIMULATOR_GPS_GPS_RECEIVER_H_
#define DRONE_SIMULATOR_GPS_GPS_RECEIVER_H_

#include <optional>

namespace drone_simulator::gps {

struct GpsFix {
  double latitude_deg = 0.0;
  double longitude_deg = 0.0;
};

class GpsReceiver {
 public:
  virtual ~GpsReceiver() = default;

  virtual std::optional<GpsFix> ReadFix() = 0;
};

}

#endif
