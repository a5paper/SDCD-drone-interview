# Decisions, assumptions and rejected ideas

Checklist form: what was chosen, its cost, and what a new feature would
break. Full wording of A/T rows is in `tradeoffs.md`. IDs are stable: never
renumber; delete a dropped row.

## Assumptions

| ID | Assumption | Conflicts with a feature that... |
|---|---|---|
| A1 | Simple physics: speed is constant or 0, no acceleration; command length clamped to 1, so a diagonal is max speed, not 1.41x | adds acceleration, inertia, wind, drag, momentum, altitude dynamics |
| A2 | Drone and world simulated in one project, separated in code (`sim/drone` vs `sim/world`) | splits them into processes, or lets the world change drone state directly |
| A3 | The world does not matter much: minimal effort on the camera and scene | adds world detail (traffic lights, weather, terrain, 3D) for its own sake |
| A4 | Built and run on Ubuntu 24.04 (also WSL) and Windows 10/11 with MSVC (VS 2022 or 2026) | needs macOS, MinGW/Clang-cl, another toolchain or hardware |

## Tradeoffs

| ID | Decision | Cost | Conflicts with a feature that... |
|---|---|---|---|
| T1 | JPEG per frame over own chunk protocol, not H.264/FFmpeg | 5-10x bandwidth | adds a video codec, ffmpeg, libav, keyframes |
| T2 | Telemetry is a separate stream, matched to frames by `frame_id` | small matching buffer | puts telemetry inside video chunks, or makes it depend on video |
| T3 | UDP everywhere; GCS resends commands | no delivery guarantee | adds TCP, acknowledgements, ordered delivery |
| T4 | No retransmission; incomplete frames dropped | lower frame rate under loss | adds NACK/ARQ, buffering of old frames |
| T5 | FEC as a strategy (none / XOR), chosen in `drone.ini`, each chunk names its scheme | 8-23 % bandwidth | hard-codes one FEC, or changes the chunk format without the scheme field |
| T6 | Every datagram under the MTU (no IP fragmentation) | more chunks per frame | sends large datagrams, raises `kMaxChunkPayloadBytes` past the MTU |
| T7 | Protobuf for all messages, C++ and Python; 3.21.12 pinned on both OSes (apt; vcpkg `overrides`), Python `protobuf==4.21.12` | a few bytes per message; an old library compiled from source on Windows | hand-packs bytes, uses JSON on the wire |
| T8 | Single-threaded 60 Hz loop; video every 2nd tick (30 fps), telemetry every 6th (10 Hz); `LoopClock` waits for a deadline (previous deadline + period) on both OSes, `timeBeginPeriod(1)` on Windows | render must fit the tick; rare 50-130 ms gaps from the host scheduler | adds threads, locks, or a rate not dividing 60 |
| T9 | Cars as a structure of arrays, never created/deleted, index = persistent ID | cars cannot turn | spawns/deletes cars, lets them turn or change lanes, reorders the arrays |
| T10 | Cars ignore each other at crossings (drive through) | unrealistic | adds traffic lights, queues, collisions (and the design doc assumes real cars) |
| T11 | Two kinds of config: per-run settings in `drone.ini` (unique keys), design constants in `config.h` | two places | puts deployment settings in code, constants in the INI, or reuses an INI key name |
| T12 | GCS in tkinter, frames shown as PPM, no Pillow | Tk limits | adds Qt, web UI, Pillow |
| T13 | GPS behind `GpsReceiver` (strategy + injection); `GpsStatus` oneof | one interface | reads position from `World` directly in telemetry, drops the no-fix state |
| T14 | No drone marker in the video; the GCS draws overlays (`[gcs.ui] overlays`) | none | draws HUD items in the simulator frame |
| T15 | GCS polls sockets from the Tk loop, no threads | ~5 % idle CPU, up to 5 ms delay | adds threads to the GCS (the design doc plans one worker for detection only) |
| T16 | Commands latched and resent every 100 ms; backoff to 1 s while LOST; 3 stops on exit; drone stops after `command_timeout_ms` (1500 ms) without a valid command | up to 1.5 s (15 m) of flight after the GCS dies | sends one-shot commands, removes resend, relies on a command being received once, or sets `command_timeout_ms` <= `backoff_max_ms` (stop/go while LOST) |
| T17 | Restarts detected from ids, not configured | heuristics (`restart_gap_*`, `stale_ms`) | resets ids, or changes `frame_id`/`sequence` meaning |
| T18 | Unit tests cover the video path (FEC parity, `VideoStreamer` chunking, `FrameAssembler` reassembly): it is what actually travels between the drone and the GCS, the most important data, and the most algorithmically intense code; plus a few `Drone` command-ordering tests (T19, user's request), `UdpSocket` tests and `LoopClock` tests | the rest of sim, telemetry and config have no unit tests | changes the chunk format, FEC or reassembly without updating `DroneSimulator/tests/` and `GCS/tests/` |
| T19 | Commands carry `timestamp_ms` (GCS wall clock, uint64 Unix ms); `Drone::HandleCommand` ignores a command older than `last_command`, equal timestamps apply; `Poll` unchanged; no resync | an older command arriving after a newer one in the same 16 ms tick still wins (the next resend fixes it within 100 ms); a GCS clock stepping backwards makes the drone ignore commands until it catches up | moves ordering out of `HandleCommand`, sends commands without a timestamp, switches to a GCS clock that restarts (monotonic) without adding a resync rule |

## Windows port

| ID | Decision | Cost | Conflicts with a feature that... |
|---|---|---|---|
| W1 | vcpkg manifest (`vcpkg.json`) with a pinned `builtin-baseline`, triplet `x64-windows-static-md` (static libraries, dynamic CRT) | first install builds from source (about 7 min cold); vcpkg comes from Visual Studio, no separate clone | adds a dependency without a `vcpkg.json` entry, or needs DLLs next to the exes |
| W2 | `opencv4` with `default-features: false`, features `jpeg, highgui, win32ui, intrinsics, thread`; no `dnn` | OpenCV 4.14 on Windows vs 4.6 on Ubuntu; only `imencode`, drawing and a `highgui` window may be used | uses OpenCV modules outside those features (dnn would pull its own protobuf); the window needs `win32ui` |
| W3 | Separate per-OS build dir and venv: `build-windows/` + `GCS/.venv-windows/` vs `build/` + `GCS/.venv/`; `GCS/protocols.py` picks the dir by `sys.platform`; `Ninja Multi-Config` preset so Debug and Release share one dir | two dirs and two venvs in one checkout | shares one build dir or venv between WSL and Windows |
| W4 | PowerShell twins (`install-deps.ps1`, `compile.ps1`, `test.ps1`) beside the `.sh` scripts; `.gitattributes` keeps `*.sh` LF and `*.ps1` CRLF (autocrlf broke `compile.sh` in WSL) | every script change is made twice | changes a `.sh` without its `.ps1` twin, or the reverse |
| W5 | `install-deps.ps1` checks first and installs only after an explicit `y` (winget, Visual Studio Installer, `py -3.12`); `-WhatIf` prints the commands | the install path is untested (only the checks and `-WhatIf` ran) | installs anything silently |
| W6 | Windows code stays in two .cpp files (`network/udp_socket.cpp` Winsock helpers, `timing/loop_clock.cpp` `timeBeginPeriod`), plus the socket handle typedef in `udp_socket.h` (the `LoopClock` destructor is declared on both OSes and empty on Linux), and the portable `network::ErrorText` replacing `std::strerror` | `#ifdef _WIN32` in two places | spreads `#ifdef _WIN32` or Win32 headers through the simulator |
| W7 | Windows runs Python 3.12 (`py -3.12`), like Ubuntu 24.04; Python pins (numpy 2.2.6 etc.) unchanged | Python 3.14 is not supported by the pins | bumps the pins or needs another Python |

## Other decisions

- Only `VideoStreamer` knows the encoding; H.264 would change it and the GCS
  decoder only.
- UART, I2C and SPI are explained, never implemented; a UART GPS would be one
  more `GpsReceiver`.
- 60 Hz loop because 30 fps needs a rate divisible by 30.
- Each chunk carries `fec_scheme` and `fec_group_size`; the receiver needs no
  config.
- Telemetry `last_command` is the last *accepted* command: the delivery ack
  UDP lacks.
- Cars fill every lane from `kRandomSeed` and wrap; buildings are seeded per
  block in the renderer, nothing stored. Runs repeat exactly.
- Simulator preview window stays (user's choice), `[simulator] preview`.
- GCS link badge: LIVE <= 200 ms, STALE <= 1 s, LOST, from the time since the
  last telemetry. Arrows latch; opposite arrow reverses; clicking a latched
  arrow clears it. Slider on 10 % notches.
- `FrameAssembler` restarts a frame when `chunk_count` changes for the same
  `frame_id` (a forged count once crashed the GCS).
- `LoopClock` is deadline-based on both OSes; frame-gap bar
  mean 33.3 +/- 0.5 ms, typical p99 ~3 ms; host stalls accepted.
- `main.cpp` builds `World`, GPS and `Drone`; `Simulator` only holds
  references. The command timeout counts simulated time in `Drone::Update`.
- Windows switch is `-Clean`, not `--clean`.

## Rejected by the user (do not propose unprompted)

FFmpeg/libav; telemetry inside video chunks; JSON or protobuf text for config;
hardware emulation (MIPI CSI-2, I2C HAL); more simulator polish; licence
headers and closing namespace/guard comments; large unrequested multi-file
drops; a plain `sim::Command` struct replacing the protobuf command in `sim/`;
a clean SIGINT shutdown of the simulator.

## Detection and tracking design (Docs/proposals/car-tracking-design.tex)

| ID | Decision | Conflicts with a feature that... |
|---|---|---|
| D1 | Model and tracker run in the GCS, also with a real drone; drone compute is unknown | runs detection on the drone or in the simulator (onboard detection is out of scope) |
| D3 | Pretrained Apache-2.0 detector (YOLOX-Nano, COCO weights), ONNX Runtime, no fine-tuning; AGPL (Ultralytics) and VisDrone-trained (CC BY-NC-SA 3.0) weights rejected | uses Ultralytics/AGPL code, non-commercial weights, or a colour-rule detector for real cars |
| D4 | SORT in world metres from GPS; ground-shift fallback without a fix | tracks only in pixels, ignores drone motion |
| D10 | Worker thread, mailbox of one frame (newest wins); drawn as a `tracks` overlay | runs detection in the Tk loop |
| D12 | Testing: unit tests on made-up boxes; scoring against the simulator's true car IDs (ID switches, missed cars, false cars) on a debug port; live run under `tc netem` loss with FEC on and off | |
| D13 | Model scans the whole frame every frame, one fixed score threshold; the model has no memory or motion, the filter's prediction keeps IDs stable | adds crops/search areas, or lets the model assign IDs |
| D14 | One extended Kalman filter per car, constant turn rate and velocity (CTRV), world metres; drone heading error enters as more measurement noise for boxes far from the image centre | uses a plain constant-velocity Kalman filter, or tracks in pixels |
| D15 | Gate is the EKF's own uncertainty ellipse around the prediction, capped at ~3 m (queued cars 6 m apart); Hungarian pairing over all cars, distance scaled by uncertainty | matches by IoU, per car greedy, or uncaps the gate |
| D16 | Track states: New (3 matches in a row to become Tracked), Tracked, Lost (coasts, gate grows), Deleted when the gate reaches the cap; only Tracked cars drawn; lost frames change no state | adds per-case time limits or draws unconfirmed tracks |
| D17 | No appearance check; DeepSORT is the upgrade if look-alikes swap after long gaps | adds colour/size or a re-ID network |

## Known gaps (accepted, not planned)

No packet authentication; ~5 % idle CPU from polling; `NetworkConfig` holds
`preview`; unit tests cover only the video path, `Drone` command ordering,
`UdpSocket` and `LoopClock`; drone top speed (10 m/s) is below fast car speeds
(15 m/s); Windows: "Video/Telemetry send failures" stay 0 with no GCS because a
connected UDP send to a closed port never fails there (Linux counts
`ECONNREFUSED`), the install path of `install-deps.ps1` is untested, VS 2022 is
accepted but untested, and a manual keyboard/slider check of the GCS on Windows
is pending; a Windows `recv` of an oversized datagram fails with `WSAEMSGSIZE`
(logged, dropped) where Linux truncates it; `HoldsRateOverManyTicks` fails only
if all 3 of its timing windows hit a scheduler stall above 5 ms; the simulator
draws coloured rectangles while the design assumes car sprites; the simulator
has no bridges, queues or turns that the design's tests describe.
