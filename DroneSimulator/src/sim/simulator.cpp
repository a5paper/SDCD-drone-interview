#include "sim/simulator.h"

namespace drone_simulator {

Simulator::Simulator(world::World& world, drone::Drone& drone)
    : world_(world), drone_(drone) {}

void Simulator::HandleCommand(
    const drone_protocol::VelocityChangeCommand& command) {
  drone_.HandleCommand(command, world_.GetState().elapsed_time_s);
}

void Simulator::Update(double delta_time) {
  drone_.Update(delta_time, world_.GetState().elapsed_time_s);

  const drone::Vector2 velocity = drone_.GetVelocity();
  world_.Update(velocity.x, velocity.y, delta_time);
}

const drone::Drone& Simulator::GetDrone() const { return drone_; }

const world::World& Simulator::GetWorld() const { return world_; }

}
