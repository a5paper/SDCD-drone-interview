#include "sim/drone.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <optional>

#include "config/config.h"

namespace drone_simulator::drone {
namespace {

constexpr double kTickS = config::kTickPeriodS;

class NoGps : public gps::GpsReceiver {
 public:
  std::optional<gps::GpsFix> ReadFix() override { return std::nullopt; }
};

drone_protocol::VelocityChangeCommand MakeCommand(double velocity_x,
                                                  uint64_t timestamp_ms) {
  drone_protocol::VelocityChangeCommand command;
  command.set_velocity_x(velocity_x);
  command.set_timestamp_ms(timestamp_ms);
  return command;
}

class DroneTest : public ::testing::Test {
 protected:
  NoGps gps_;
  Drone drone_{gps_};
};

TEST_F(DroneTest, NewerCommandApplies) {
  drone_.HandleCommand(MakeCommand(1.0, 1000), 0.0);
  drone_.HandleCommand(MakeCommand(0.5, 1100), 0.0);

  EXPECT_EQ(drone_.GetState().last_command.timestamp_ms(), 1100u);
  EXPECT_DOUBLE_EQ(drone_.GetState().speed_mps, 0.5 * config::kMaxSpeedMps);
}

TEST_F(DroneTest, OlderCommandIsIgnored) {
  drone_.HandleCommand(MakeCommand(1.0, 1000), 0.0);
  drone_.HandleCommand(MakeCommand(-1.0, 900), 0.0);

  const DroneState& state = drone_.GetState();
  EXPECT_EQ(state.last_command.timestamp_ms(), 1000u);
  EXPECT_DOUBLE_EQ(state.last_command.velocity_x(), 1.0);
  EXPECT_DOUBLE_EQ(state.direction.x, 1.0);
  EXPECT_DOUBLE_EQ(state.speed_mps, config::kMaxSpeedMps);
}

TEST_F(DroneTest, EqualTimestampApplies) {
  drone_.HandleCommand(MakeCommand(1.0, 1000), 0.0);
  drone_.HandleCommand(MakeCommand(0.0, 1000), 0.0);

  EXPECT_DOUBLE_EQ(drone_.GetState().speed_mps, 0.0);
}

TEST_F(DroneTest, OlderCommandDoesNotRefreshTimeout) {
  drone_.HandleCommand(MakeCommand(1.0, 1000), 0.0);
  drone_.HandleCommand(MakeCommand(1.0, 900), 1.0);
  drone_.Update(kTickS, config::kDefaultCommandTimeoutS + kTickS);

  EXPECT_DOUBLE_EQ(drone_.GetState().speed_mps, 0.0);
}

}
}
