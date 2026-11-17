#!/usr/bin/env python3
"""Ground Control Station: live video, telemetry and keyboard/mouse control.

Usage:
    GCS/.venv/bin/python GCS/main.py [--config FILE] [--no-fec]

Close the window, press q, or press Ctrl+C in the terminal to stop. On exit it
prints how many frames were shown of those sent, for the packet-loss test.
"""

import argparse
import signal
import sys
import tkinter as tk

import config
from app import App
from services.telemetry_receiver import TelemetryReceiver
from services.video_receiver import VideoReceiver


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--config", default=str(config.DEFAULT_PATH))
    parser.add_argument("--no-fec", action="store_true",
                        help="ignore parity chunks (no loss recovery)")
    args = parser.parse_args()
    settings = config.load(args.config)
    link = settings["gcs.link"]
    bind_address = link["bind_address"]
    network = settings["network"]
    video_port = network.getint("video_port")
    telemetry_port = network.getint("telemetry_port")
    restart_gap = link.getint("restart_gap_frames")
    # A stream silent for longer than the link's LOST limit that comes back
    # with a lower id is a restarted simulator.
    silence_ms = link.getint("stale_ms")
    try:
        video = VideoReceiver(bind_address, video_port, not args.no_fec,
                              restart_gap, silence_ms)
        telemetry = TelemetryReceiver(bind_address, telemetry_port,
                                      link.getint("telemetry_kept"),
                                      restart_gap, silence_ms)
    except OSError as error:
        sys.exit(f"Cannot listen on UDP {bind_address} ports {video_port} "
                 f"and {telemetry_port} ({error.strerror}). Is another "
                 f"receiver running?")
    print(f"Listening on UDP {bind_address}: video {video_port}, telemetry "
          f"{telemetry_port}")

    root = tk.Tk()
    app = App(root, settings, video, telemetry)
    # Tk does not see SIGINT; the periodic poll lets Python run this handler.
    # It can run in the middle of a poll, so it only schedules the destroy for
    # after the current callback, never destroys widgets that are in use.
    signal.signal(signal.SIGINT, lambda *_: root.after_idle(root.destroy))
    root.mainloop()

    shown, sent = app.state.shown, app.state.sent
    percent = 100.0 * shown / sent if sent else 0.0
    print(f"Showed {shown} of {sent} frames sent ({percent:.1f} %), "
          f"{video.assembler.recovered} rebuilt with FEC, "
          f"{video.rejected} bad datagrams, "
          f"{video.assembler.restarts} simulator restarts")
    tracker = telemetry.tracker
    print(f"Telemetry: {tracker.lost} of {tracker.expected} packets lost, "
          f"{telemetry.rejected} bad datagrams")


if __name__ == "__main__":
    main()
