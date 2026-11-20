---
name: sdcd-codebase
description: Project knowledge and working rules for the SDCD-for-interview repo (a C++ drone simulator and a Python ground control station that talk over UDP, plus a car detection and tracking design). Use this skill whenever you need to find, read, explain, search, test or change anything in this repo, such as "where is X", "how does the video/telemetry/command path work", "what does this constant mean", "why was it built this way", "what is left to do", "how did we test Y", or "which file do I touch for Z", even if the user does not name the skill. It holds the decisions and their costs, the measured numbers, the project status, the conventions and how this user likes to work, so you do not have to rediscover them.
---

# SDCD codebase

Job-interview assignment: a **Drone Simulator** (C++, server) and a **Ground
Control Station** (GCS, Python + tkinter + OpenCV, client) over UDP. Car
detection and tracking is a design doc only. The user must defend every
choice in an interview, so *why* matters as much as *where*, and numbers
must be measured, never estimated.

## Sources of truth

| What | Where |
|---|---|
| Requirements, status, open items | `references/assignment.md` |
| Data flow, environment, measured numbers | `references/code-map.md` |
| Decisions (A/T/W/D), rejected ideas, known gaps | `references/decisions.md` |
| Full wording of assumptions and tradeoffs (user's text) | `references/tradeoffs.md` |
| Detection and tracking design | `Docs/proposals/car-tracking-design.tex` |
| Design constants | `DroneSimulator/src/config/config.h` |
| Per-run settings (both nodes) | `config/drone.ini` |
| Wire formats | `protocols/*.proto` |
| History | `git log` |

The user edits files at the same time as you and sometimes discards work on
purpose: re-read before editing; if your work is gone, ask, do not restore.

## The system

Simulator: single-threaded 60 Hz loop (`main.cpp`). Polls velocity commands
(UDP 5600), moves the drone and ~4,500 cars, renders a 640x640 top-down
frame (10 px/m, drone at centre) every 2nd tick (30 fps), JPEG-encodes,
chunks with XOR parity, sends to 5602; `Telemetry` every 6th tick (10 Hz) to
5601. GCS: polls both sockets from the Tk loop every 5 ms (no threads),
reassembles frames, pairs each with telemetry by `frame_id`, resends the
latched command every 100 ms.

Assumptions code relies on: units in names (`kMaxSpeedMps`); x north, y
east; commands `(vx, vy)` in [-1, 1], length clamped to 1; drone stops after
`command_timeout_ms` (1500, must stay above `backoff_max_ms`); a car's index
is its persistent ID; newest data wins, no retransmission; every INI key
unique (the C++ loader ignores sections); Python never copies C++ constants.

Search: exclude `build/`, `build-windows/`, `vcpkg_installed/`, `GCS/.venv*`,
`__pycache__/`. Generated proto code lives in the build dir; never edit it.

## Working with this user

- Short commands ("next", "fix it"); you write the code, they decide.
- Open design choices: AskUserQuestion, 2-4 options, recommended first, each
  with benefit and cost. "Why?" gets a plain answer before re-asking. UI
  change: simple wireframe in the scratchpad, opened with
  `explorer.exe "$(wslpath -w FILE)"`, before asking.
- Doc edits: before/after per section, accept or reject each.
- Never commit or push unless asked. Commit name: title-only
  `type(scope) message`, no trailer.
- Be honest about leftovers and choices made without asking.

## Conventions

- Smallest change that meets the requirement; reuse before writing; record
  minor architecture changes in `references/decisions.md`.
- C++ Google style (80 cols, `kCamelCase` constants, `[[nodiscard]]` on error
  returns); no licence headers, no closing namespace/`#endif` comments;
  guards `DRONE_SIMULATOR_<MODULE>_<FILE>_H_`; `-Wall -Wextra -Wpedantic`
  clean (`/W4` on MSVC).
- No wrapped nested calls: name the inner result in a local first.
- No magic numbers: `config.h`, `drone.ini`, `GCS/constants/`.
- Comments only when non-obvious.
- Python: 80 cols; components take `render(state)`, report via callbacks;
  only `App` changes state.
- Fail loudly at startup, degrade quietly at runtime.
- Verify before "done": `clang-format`, `cpplint`, `pyflakes`,
  `pycodestyle` from a scratch venv (never `GCS/.venv`); warning-free build;
  `bash test.sh`; live run; failure path. Each `.sh` change needs its `.ps1`
  twin.
- Docs move with code: README, `references/tradeoffs.md`, decisions,
  status (only after a real test), measured numbers.

## Gotchas

- The user's own simulator/GCS may be running: check
  `ss -ulpn | grep -E '5600|5601|5602'`, use other ports via `--config`.
- Never `pkill -f`; kill by saved PID.
- `sudo` (`tc netem`) and `git push` are the user's to run.
- Headless: `QT_QPA_PLATFORM=offscreen`, `preview = false`, `timeout`.
- Build: `bash compile.sh [Debug|Release] [--clean]` (also builds
  `Docs/proposals/*.tex`; PDFs are git-ignored); never with sudo. Windows:
  `powershell -ExecutionPolicy Bypass -File .\compile.ps1` (and
  `install-deps.ps1`, `test.ps1`, `run.ps1`); clone to a short path.
