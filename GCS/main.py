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
from constants import settings as names
from constants.protocol import MAX_PERCENT, STOP_COMMAND
from app import App
from services.command_sender import CommandSender
from services.telemetry_receiver import TelemetryReceiver
from services.video_receiver import VideoReceiver


def open_or_exit(stream: str, receiver_class, address: str, port: int,
                 *options):
    """Returns receiver_class(address, port, *options); exits with a message
    naming the stream and port if the port cannot be bound."""
    try:
        return receiver_class(address, port, *options)
    except OSError as error:
        sys.exit(f"Cannot listen for {stream} on UDP {address}:{port} "
                 f"({error.strerror}). Is another receiver running?")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--config", default=str(config.DEFAULT_PATH))
    parser.add_argument("--no-fec", action="store_true",
                        help="ignore parity chunks (no loss recovery)")
    args = parser.parse_args()
    settings = config.load(args.config)
    link = settings[names.LINK]
    address = link[names.BIND_ADDRESS]
    network = settings[names.NETWORK]
    video_port = network.getint(names.VIDEO_PORT)
    telemetry_port = network.getint(names.TELEMETRY_PORT)
    # A stream silent for longer than the link's LOST limit that comes back
    # with a lower id is a restarted simulator.
    silence_ms = link.getint(names.STALE_MS)
    frame_gap = link.getint(names.RESTART_GAP_FRAMES)
    packet_gap = link.getint(names.RESTART_GAP_PACKETS)
    kept = link.getint(names.TELEMETRY_KEPT)
    video = open_or_exit("video", VideoReceiver, address, video_port,
                         not args.no_fec, frame_gap, silence_ms)
    telemetry = open_or_exit("telemetry", TelemetryReceiver, address,
                             telemetry_port, kept, packet_gap, silence_ms)
    print(f"Listening on UDP {address}: video {video_port}, telemetry "
          f"{telemetry_port}")
    simulator = network[names.SIMULATOR_ADDRESS]
    command_port = network.getint(names.COMMAND_PORT)
    try:
        sender = CommandSender(simulator, command_port)
    except OSError as error:
        sys.exit(f"Cannot send commands to UDP {simulator}:{command_port} "
                 f"({error}). Check simulator_address.")
    print(f"Sending commands to UDP {simulator}:{command_port}")

    try:
        root = tk.Tk()
    except tk.TclError as error:
        sys.exit(f"Cannot open the GCS window ({error}). Run it on a desktop "
                 f"session, or check DISPLAY.")
    app = App(root, settings, video, telemetry, sender)
    # Tk does not see SIGINT; the periodic poll lets Python run this handler.
    # It can run in the middle of a poll, so it only schedules the destroy for
    # after the current callback, never destroys widgets that are in use.
    signal.signal(signal.SIGINT, lambda *_: root.after_idle(root.destroy))
    root.mainloop()
    # Closing the GCS stops the drone.
    repeats = settings[names.COMMANDS].getint(names.EXIT_STOP_REPEATS)
    for _ in range(repeats):
        sender.send(STOP_COMMAND)

    shown, sent = app.state.shown, app.state.sent
    percent = MAX_PERCENT * shown / sent if sent else 0.0
    print(f"Video: showed {shown} of {sent} frames sent ({percent:.1f} %), "
          f"{video.assembler.recovered} rebuilt with FEC, "
          f"{video.rejected} bad datagrams, "
          f"{video.assembler.restarts} simulator restarts")
    tracker = telemetry.tracker
    print(f"Telemetry: {tracker.lost} of {tracker.expected} packets lost, "
          f"{tracker.duplicates} duplicates, {telemetry.rejected} bad "
          f"datagrams, {tracker.restarts} simulator restarts")
    print(f"Commands: {sender.sent} sent, {sender.failed} failed (normal "
          f"while the simulator is not running)")


if __name__ == "__main__":
    main()
