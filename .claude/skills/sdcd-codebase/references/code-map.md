# Data flow and measured numbers

## Data flow

```
GCS                                   Simulator (60 Hz loop, main.cpp)
command_sender  --UDP 5600-->  command_receiver -> Drone::HandleCommand
  (resend 100 ms, backoff to 1 s       Simulator::Update: Drone::Update (GPS,
   while LOST, 3 stops on exit)          battery, move), World::Update (cars)
video_receiver  <--UDP 5602--  every 2nd tick: JpegRenderer -> VideoStreamer
  (FrameAssembler, FEC decode)           (VideoChunk + XOR parity)
telemetry_receiver <--UDP 5601-- every 6th tick: MakeTelemetry -> MessageSender
App (Tk loop, 5 ms poll) -> state.py (immutable GcsState) -> components
```

Environment: Ubuntu 24.04 / WSL2 (GCC 13.3, OpenCV 4.6) and Windows 10/11
(MSVC 19.50, vcpkg, OpenCV 4.14); protobuf 3.21.12 on both, Python
`protobuf==4.21.12`, Python 3.12.

## Measured numbers

Quote these; never estimate. New measurements go here, with how they were measured.

| What | Value | How |
|---|---|---|
| Frames shown at 5 % loss: no FEC / XOR per frame / XOR groups of 5 | 53.1 / 83.6 / 93.7 % | `tc netem`, 40 s per run, GCS exit summary |
| Same through a userspace 5 % drop proxy | 54.4 / 84.7 / 93.3 % | GCS, no sudo |
| FEC bandwidth cost | about 8 % (one parity per frame), 23 % (groups of 5) | chunk counts |
| XOR rebuild correctness | every single-chunk loss rebuilt byte-exact (1,724 of 1,724; groups of 5 1,533 of 1,533; one loss in each of 3 groups 7,575 of 7,575) | captured simulator traffic |
| JPEG per frame | about 16.2 KiB (with buildings and world-wide traffic) | status print |
| Video bandwidth at 30 fps | 457-580 KiB/s | 20 s live |
| Largest video datagram | 1,426 bytes (limit 1,472) | capture |
| Command timeout stop (`command_timeout_ms` 1500) | speed 0 in telemetry 1.47 s after one command; no stop with resends every 1 s for 3.5 s | script sending one `VelocityChangeCommand`, 10 Hz telemetry read on ports 6600-6602 |
| Command reordering, real `CommandSender` through a proxy swapping every pair (older sent 30 ms after newer) | 62 commands, `last_command.timestamp_ms` never went backwards in 39 telemetry packets | script, ports 6600-6610 |
| Frame and telemetry rate, Linux, before the deadline-based loop | 590 of 590 frames (29.5 fps), 196 telemetry packets (10 Hz), 0 lost | 20 s live |
| Telemetry loss at 5 % netem | 5-7 % (no FEC on that stream) | GCS summary |
| Car update, 4,513 cars, SoA float | 1.3 us at -O3 (vectorized), 6.0 us at -O2, 42 us at -O0 | benchmark |
| Simulator memory | 0 bytes definitely or indirectly lost; totals equal at 15 s and 60 s | valgrind |
| GCS soak, 3 min | memory flat at 77.7 MB, 4,460 of 4,460 frames, commands 9.9 Hz | live |
| Command resend backoff while LOST | 100 ms, then 200, 400, 800, 1000 ms | live, simulator killed |
| Frame gaps, Linux (WSL), deadline-based loop | 3 runs of 20 s: 602 / 586 / 596 frames; mean gap 33.319 / 34.191 / 33.631 ms; p99 of \|gap - 33.333\| 0.804 / 2.166 / 1.116 ms; max gap 62 / 135 / 125 ms | Release build, first chunk of each `frame_id` timed by a Python receiver on the same host, preview off |
| Frame gaps, Windows Release, deadline-based loop + 1 ms timer, console in the foreground | 8 runs of 20 s: 592-602 frames; mean gap 33.333-33.869 ms; p99 of \|gap - 33.333\| 1.848-10.756 ms (typical 2.3-2.9); max gap 51-134 ms | same method |
| Frame-spacing target | mean gap 33.3 ± 0.5 ms, typical p99 about 3 ms; met by the typical run; Windows run 8 (33.87 ms mean, p99 10.8 ms) and Linux run 2 (34.19 ms mean) are outside it, from host scheduler stalls | Windows and Linux runs above |
| Frame gaps, Windows, before: loop timed from "now" | without the 1 ms timer: 322 frames in 20 s, mean gap 62.440 ms, p99 31.780 ms, max 157.388 ms; with it: 541-557 frames, mean 36.0-37.1 ms, p99 3.5-86.8 ms | same method, 1 + 4 runs |
| Host scheduler noise, Windows | bare `sleep_until` loop (1200 sleeps at 16.667 ms, 1 ms timer): max overshoot 99.5 / 96.6 / 86.8 ms, 6-11 sleeps over 10 ms; without the 1 ms timer: 110 ms max, 503 sleeps over 10 ms; busy-spin loop: one 18.6 ms stall (4 over 10 ms); WSL on the same host also shows about 100 ms gaps | standalone C++ program, no simulator |
| Live run on Windows, simulator + GCS over loopback (preview off) | 30.3-30.5 fps (277 frames / 9.15 s, 189 / 6.2 s); frames shown 100.0 % (277/277, 367/367, 189/189), one cold first run 96.2 % (427/444); telemetry 0 of 93 / 122 / 63 / 148 packets lost, 10.2 Hz; commands 9.2-9.4 Hz (Linux soak 9.9 Hz) | GCS exit summary, `fec = xor` |
| Video bandwidth, Windows | 472.7 KiB/s mean (min 439.9, max 544.8; 38 prints, 3 s apart); JPEG mean 15.76 KiB per frame | simulator status print (FEC parity not counted) |
| Command timeout stop, Windows | speed 0 in GCS-read telemetry 1.5-1.6 s after the last command (timeout 1500 ms plus up to one 100 ms telemetry period) | synthetic key events through the real GCS, sender silenced at 8.2 s |
| Steering, Windows | Up gave speed 5.0 m/s, Right a diagonal, `=` 5.1 m/s, Down; telemetry `last_command` echoed each command; badge LIVE from 0.5 s | same live run, synthetic key events |
| Second simulator on the same ports, Windows | exit code 1, "Failed to open command socket: Only one usage of each socket address..." | live |
| GCS closed for 20+ s, Windows | status print kept coming every 3 s; video and telemetry send failures stayed 0 (see UDP everywhere); restarted GCS LIVE after 0.53 s, 189 of 189 frames | live |
| Windows setup, cold vcpkg cache | about 7 min for the first `vcpkg install` (OpenCV about 5) on the development machine | first full build, no binary cache |
| Windows setup, warm vcpkg cache | `install-deps.ps1` on a complete machine 11.5 / 11.3 s; fresh copy at a 38-character path: `install-deps.ps1` 52.3 s, `compile.ps1` (full Debug build) 26 s, `test.ps1` 5.3 s; fresh `git clone` builds in 19 s | `Get-Date` around each script |
| Unit tests | 23 GoogleTest (17 earlier + 4 socket + 2 loop clock) + 12 pytest pass, on Ubuntu and Windows | `bash test.sh`, `test.ps1` |
