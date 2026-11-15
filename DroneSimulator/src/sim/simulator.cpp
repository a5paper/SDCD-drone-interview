#include "sim/simulator.h"

namespace drone_simulator {

void Simulator::HandleCommand(
    const drone_protocol::VelocityChangeCommand& command) {
  drone_.HandleCommand(command);
}

void Simulator::Update(double delta_time) {
  drone_.Update(delta_time);

  const drone::DroneVelocity& velocity = drone_.GetState().velocity;
  world_.Update(velocity.x, velocity.y, delta_time);
}

const drone::Drone& Simulator::GetDrone() const { return drone_; }

const world::World& Simulator::GetWorld() const { return world_; }

}
