# SDCD-for-interview

## How to compile
1. Use a Ubuntu 24 LTS or a windows computer the same OS on WSL.
2. `bash install-deps.sh` (asks for sudo itself; also creates the GCS's Python venv in `GCS/.venv` from `GCS/requirements-dev.txt`, which adds pytest to `GCS/requirements.txt`)
3. `bash compile.sh [Debug|Release] [--clean]` (Debug by default; do not use sudo, or `build/` becomes root-owned)
4. Run the simulator: `./build/DroneSimulator/DroneSimulator`
5. Run the Ground Control Station: `GCS/.venv/bin/python GCS/main.py [--no-fec]`

GCS controls:
- Arrow pad or WASD / arrow keys: latch a direction (the opposite one reverses it; up/down and left/right combine into a diagonal). Clicking a latched (grayed) arrow clears it; keys never clear, since a held key repeats. ■ or Space: stop.
- Intended speed: drag the slider (10 % steps), or - / + buttons and the `-` / `=` keys (1 %); `_` / `+` (Shift) move 10 %.
- q, closing the window or Ctrl+C quits; the GCS then sends a final stop and prints video, telemetry and command statistics.

Settings live in `config/drone.ini`, read by both nodes. The simulator's own preview window is switched by `preview = true|false` in `[simulator]`; set it to `false` to run the simulator without a display. To run the nodes on two machines, copy it, set `simulator_address` and `gcs_address`, and pass `--config PATH` to both programs. To run the nodes on two machines, copy it, set `simulator_address` and `gcs_address`, and pass `--config PATH` to every program.

## Debugging with GDB
- Debug builds (the default) use `-g3 -O0`.
- Terminal: `gdb ./build/DroneSimulator/DroneSimulator`, e.g. `break drone_simulator::drone::Drone::HandleCommand`, then `run`.
- VS Code: install the C/C++ extension (`ms-vscode.cpptools`) and press F5. `.vscode/launch.json` builds with the "build (Debug)" task first.

## Testing under packet loss
`tc netem` drops packets on the loopback interface, so all three streams lose packets as on a bad link.
1. Turn on 5 % loss: `sudo tc qdisc add dev lo root netem loss 5%` (if the qdisc kind is unknown, run `sudo modprobe sch_netem` first).
2. Run the simulator: `./build/DroneSimulator/DroneSimulator`
3. Run the GCS for about 40 s, once with FEC and once without, and stop it with q:
   - `GCS/.venv/bin/python GCS/main.py`
   - `GCS/.venv/bin/python GCS/main.py --no-fec`
4. On exit the GCS prints `Video: showed X of Y frames sent (Z %)` and the telemetry loss. Compare Z between the two runs (the panel shows the same numbers live).
5. Turn the loss off: `sudo tc qdisc del dev lo root` (check with `tc qdisc show dev lo`).

Measured at 5 % loss, 40 s per run: 53.1 % of frames shown without FEC, 83.6 % with one XOR parity chunk per frame, 93.7 % with `fec_group_size = 5` in `config/drone.ini`. The loss rate changes with `sudo tc qdisc change dev lo root netem loss 10%`.

## Unit tests
`bash test.sh` (after `compile.sh`) runs both suites. They cover the video path: FEC parity and frame reassembly.
- C++ (GoogleTest, `ctest`): `XorFec`/`NoFec` parity, including a short last chunk and rebuilding any one lost chunk; `VideoStreamer` chunk indices, sizes and parity order, read back from a loopback socket on UDP 5699.
- GCS (pytest): `FrameAssembler` and the FEC decoders rebuilding one lost chunk per group, dropping a frame with two losses in a group, ignoring parity with `--no-fec`, superseding an incomplete frame, and resyncing after a restart.

## Code style
C++ follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
- Format: `clang-format -i $(find DroneSimulator/src -name '*.h' -o -name '*.cpp')` (config in `.clang-format`).
- Lint: `cpplint --recursive DroneSimulator/src` (config in `DroneSimulator/CPPLINT.cfg`).
- Both tools install with `pip install clang-format cpplint` (in a venv on Ubuntu 24).

## Project layout
```
config/drone.ini           settings for both nodes: [network], [video], [gcs.*]
protocols/                 protobuf messages (drone_protocol library)
DroneSimulator/src/
  main.cpp                 main loop: commands, update, video, telemetry, status
  config/                  config.h (all constants), INI and network config loaders
  timing/                  LoopClock and IntervalTimer (with config/: drone_core library)
  sim/                     drone, world, road grid, simulator step (drone_sim library)
  gps/                     GpsReceiver interface, SimulatedGpsReceiver (World to lat/lon)
  render/                  top-down renderer, JPEG encoding (drone_render library)
  network/                 UDP socket, command receiver, message sender, video streamer,
                           FEC strategies
                           (drone_network library)
DroneSimulator/tests/      GoogleTest: FEC parity, video chunking
GCS/                       Ground Control Station (Python, tkinter + OpenCV)
  main.py                  startup, exit summary
  app.py                   owns the state, polls the streams, sends commands
  state.py                 immutable state and the pure functions that change it
  config.py, protocols.py  drone.ini loader; the generated protobuf imports
  constants/               protocol facts, keys, UI text, UI layout, INI names
  components/              video view, telemetry panel, command strip (one per file)
  components/overlays/     drawn on top of the video (crosshair); chosen in drone.ini
  services/                UDP helpers, video and telemetry receivers, command sender
  tests/                   pytest: frame reassembly and FEC recovery
  requirements.txt         Python packages for GCS/.venv
  requirements-dev.txt     requirements.txt plus pytest
test.sh                    runs the C++ and GCS unit tests
```

## Assumptions
For the sake of the first Task I made several assumptions:
1. Simple physics — I assume the drone moves at either a constant speed or 0, since I done want to deal with acceloration.
   The command (Vx, Vy) is split into a unit direction and a speed: speed = min(length, 1) × max speed. A diagonal command therefore moves at max speed, not √2 × max speed.
2. World Simulation — For the sake of this simulation I am making it all in the same project and creating an in code seperation between the drone simulation and the world simulation.
3. The world doesn't particularly matter — Since I want to simulate a drone, and not a lively world, I am going to put minimal effort into the camera, much like the question asks.
4. Build environemnt — This project is built on WSL ubuntu 24.