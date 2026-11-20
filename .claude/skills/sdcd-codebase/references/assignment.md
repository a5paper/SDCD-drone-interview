# The assignment (SW_Assigment_2026.pdf, not in the repo)

- **Task 1, simulator (C++ chosen):** velocity commands Vx, Vy in [-1, 1],
  simultaneous inputs; generated top-down video reacting in real time; video
  over UDP, designed for loss and bandwidth limits; telemetry (speed, battery
  draining over time/distance, last command) synchronized with video,
  protocol choice justified.
- **Task 2, GCS (Python chosen):** simple UI, live UDP video, real-time
  telemetry.
- **Challenge:** moving cars, real-time detection, persistent IDs. Required:
  a detailed design proposal, defended in the interview. Implementation
  optional.
- **Submission:** public GitHub repo; README with build, deps and run steps
  for both nodes; 1-2 min recording that also mentions UART
  (point-to-point), I2C (addresses) and SPI (faster).

## Status

Change a row only after a real test.

| Item | Status |
|---|---|
| Task 1 simulator | Done |
| Task 2 GCS | Done |
| Detection design | Done (`Docs/proposals/car-tracking-design.tex`), not implemented |
| Unit tests (video path, `Drone` ordering, socket, clock) | Done: 23 GoogleTest + 12 pytest |
| Windows port | Done and tested from a clone; pending: `install-deps.ps1` install path, VS 2022, manual GCS keyboard check |
| README: build/run both nodes | Partly: TeX Live packages not listed in README |
| `references/tradeoffs.md` matches final design | Open |
| README: chunk protocol and ports | Open |
| README: UART, I2C, SPI section | Open |
| Screen recording | Open |
| Fresh-clone test, push to public GitHub | Open (user pushes) |

## Later (optional)

- Battery drains with distance as well as time.
- Implement detection and tracking in the GCS from the design doc.
