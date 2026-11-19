# SDCD-for-interview

## How to compile
Ubuntu 24.04 LTS (also on WSL) and Windows 10/11 (native, no WSL) are supported.

### Ubuntu 24.04
1. Use Ubuntu 24 LTS, or Windows with WSL running it.
2. `bash install-deps.sh` (asks for sudo itself; also creates the GCS's Python venv in `GCS/.venv` from `GCS/requirements-dev.txt`, which adds pytest to `GCS/requirements.txt`)
3. `bash compile.sh [Debug|Release] [--clean]` (Debug by default; do not use sudo, or `build/` becomes root-owned)
4. Run the simulator: `./build/DroneSimulator/DroneSimulator`
5. Run the Ground Control Station: `GCS/.venv/bin/python GCS/main.py [--no-fec]`
6. Or run the whole system with one command: `bash run.sh [Debug|Release] [--config PATH]` (Release by default; builds first if needed; the simulator runs in the background with its output in `build/simulator.log`, and closing the GCS stops it)

### Windows 10/11
1. Prerequisites: Visual Studio 2022 or 2026, Build Tools or the IDE, with the C++ workload ("Desktop development with C++"), and Python 3.12 (found with `py -3.12`). `install-deps.ps1` checks both and offers to install what is missing with winget or the VS Installer; it never installs without a `y`.
   - Clone to a short path such as `C:\src\SDCD`. A long path breaks the C++ library build (Windows 260-character limit).
   - Only VS 2026 is tested on the development machine; VS 2022 is accepted but untested.
   - The install path itself (winget and the Visual Studio Installer) is untested; only the checks and `-WhatIf` were run.
2. Setup, build and test, in PowerShell from the repo root:
   - `powershell -ExecutionPolicy Bypass -File .\install-deps.ps1` (the first run builds the C++ libraries: about 7 minutes with a cold cache on the development machine (slower machines take longer), 52 s with vcpkg's binary cache warm; it also creates `GCS\.venv-windows`)
   - `powershell -ExecutionPolicy Bypass -File .\compile.ps1 [Debug|Release] [-Clean]` (Debug by default; a full Debug build took 26 s with the libraries cached)
   - `powershell -ExecutionPolicy Bypass -File .\test.ps1` (both test suites)
   - `-ExecutionPolicy Bypass` is needed because the default Windows policy blocks `.\x.ps1`.
3. Run the simulator: `build-windows\DroneSimulator\Debug\DroneSimulator.exe` (`Release` after a Release build), then the Ground Control Station: `GCS\.venv-windows\Scripts\python.exe GCS\main.py [--no-fec]`
   - Or run the whole system with one command: `powershell -ExecutionPolicy Bypass -File .\run.ps1 [Debug|Release] [-ConfigPath PATH]` (Release by default; builds first if needed; the simulator runs hidden with its output in `build-windows\simulator.log`, and closing the GCS stops it)
4. Windows Firewall may ask about `DroneSimulator.exe` and `python.exe` (they listen on all addresses); allow private networks. No prompt appeared on the development machine, where all traffic is on loopback.
5. Windows builds go to `build-windows\` and `GCS\.venv-windows\`, so one checkout can hold both the Linux and the Windows build.

GCS controls:
- Arrow pad or WASD / arrow keys: latch a direction (the opposite one reverses it; up/down and left/right combine into a diagonal). Clicking a latched (grayed) arrow clears it; keys never clear, since a held key repeats. ■ or Space: stop.
- Intended speed: drag the slider (10 % steps), or - / + buttons and the `-` / `=` keys (1 %); `_` / `+` (Shift) move 10 %.
- q, closing the window or Ctrl+C quits; the GCS then sends a final stop and prints video, telemetry and command statistics.

Settings live in `config/drone.ini`, read by both nodes. The simulator's own preview window is switched by `preview = true|false` in `[simulator]`; set it to `false` to run the simulator without a display. If no valid command arrives for `command_timeout_ms` (optional, `[simulator]`, default 1500 ms), the drone stops; keep it above `backoff_max_ms`. To run the nodes on two machines, copy it, set `simulator_address` and `gcs_address`, and pass `--config PATH` to both programs. To run the nodes on two machines, copy it, set `simulator_address` and `gcs_address`, and pass `--config PATH` to every program.

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
`bash test.sh` (Windows: `test.ps1`, after the compile step) runs both suites. 23 GoogleTest and 12 pytest tests. They cover the video path (FEC parity and frame reassembly), command ordering, the UDP socket and the loop clock.
- C++ (GoogleTest, `ctest`): `XorFec`/`NoFec` parity, including a short last chunk and rebuilding any one lost chunk; `VideoStreamer` chunk indices, sizes and parity order, read back from a loopback socket on UDP 5699; `Drone` command ordering (a newer or equal timestamp applies, an older one is ignored and does not refresh the command timeout); `UdpSocket` (4 tests on UDP 5698: an empty receive would block, binding a port twice fails with error text, a bad address is rejected, a loopback round trip); `LoopClock` (2 tests: holds 30 Hz over many ticks, no burst of ticks after an overrun).
- GCS (pytest): `FrameAssembler` and the FEC decoders rebuilding one lost chunk per group, dropping a frame with two losses in a group, ignoring parity with `--no-fec`, superseding an incomplete frame, and resyncing after a restart.

## Code style
C++ follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
- Format: `clang-format -i $(find DroneSimulator/src -name '*.h' -o -name '*.cpp')` (config in `.clang-format`).
- Lint: `cpplint --recursive DroneSimulator/src` (config in `DroneSimulator/CPPLINT.cfg`).
- Both tools install with `pip install clang-format cpplint` (in a venv on Ubuntu 24).
