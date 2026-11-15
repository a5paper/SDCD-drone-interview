# SDCD-for-interview

## How to compile
1. Use a Ubuntu 24 LTS or a windows computer the same OS on WSL.
2. `bash install-deps.sh` (asks for sudo itself)
3. `bash compile.sh [Debug|Release] [--clean]` (Debug by default; do not use sudo, or `build/` becomes root-owned)
4. Run: `./build/DroneSimulator/DroneSimulator`

## Debugging with GDB
- Debug builds (the default) use `-g3 -O0`.
- Terminal: `gdb ./build/DroneSimulator/DroneSimulator`, e.g. `break drone_simulator::drone::Drone::HandleCommand`, then `run`.
- VS Code: install the C/C++ extension (`ms-vscode.cpptools`) and press F5. `.vscode/launch.json` builds with the "build (Debug)" task first.

## Project layout
```
protocols/                 protobuf messages (drone_protocol library)
DroneSimulator/src/
  main.cpp                 main loop: receive, update, render, status
  core/                    config.h (all constants), types.h (type aliases)
  sim/                     drone, world, simulator step (drone_sim library)
  render/                  top-down renderer, JPEG encoding (drone_render library)
  network/                 UDP socket, command receiver (drone_network library)
```

## Assumptions
For the sake of the first Task I made several assumptions:
1. Simple physics — I assume the drone moves at either a constant speed or 0, since I done want to deal with acceloration.
2. World Simulation — For the sake of this simulation I am making it all in the same project and creating an in code seperation between the drone simulation and the world simulation.
3. The world doesn't particularly matter — Since I want to simulate a drone, and not a lively world, I am going to put minimal effort into the camera, much like the question asks.
4. Build environemnt — This project is built on WSL ubuntu 24.