"""Loads config/drone.ini, the settings shared with the simulator.

Every setting is checked once at startup; a bad file exits with a message
instead of failing later in the middle of a run.
"""

import configparser
import pathlib
import sys

DEFAULT_PATH = (pathlib.Path(__file__).resolve().parents[1] / "config" /
                "drone.ini")
PORT = (1, 65535)  # Valid UDP ports.
POSITIVE = (1, sys.maxsize)
PERCENT = (0, 100)
STEP_PERCENT = (1, 100)
REQUIRED_TEXT = {
    "network": ("simulator_address", "gcs_address"),
    "gcs.link": ("bind_address",),
}
# Each key must be a whole number within its (low, high) range.
REQUIRED_INTS = {
    "network": {"command_port": PORT, "telemetry_port": PORT,
                "video_port": PORT},
    "gcs.link": {"poll_ms": POSITIVE, "restart_gap_frames": POSITIVE,
                 "telemetry_kept": POSITIVE,
                 "live_ms": POSITIVE, "stale_ms": POSITIVE},
    "gcs.commands": {"resend_ms": POSITIVE, "backoff_max_ms": POSITIVE},
    "gcs.ui": {"speed_start_percent": PERCENT,
               "speed_drag_step_percent": STEP_PERCENT,
               "speed_nudge_step_percent": STEP_PERCENT},
}


def load(path: str) -> configparser.ConfigParser:
    """Returns the parsed file; exits if the file, a section or a key is bad."""
    parser = configparser.ConfigParser()
    try:
        if not parser.read(path):
            sys.exit(f"Cannot read config file: {path}")
    except configparser.Error as error:
        sys.exit(f"{path}: bad INI file: {error}")
    sections = set(REQUIRED_TEXT) | set(REQUIRED_INTS)
    missing = [name for name in sorted(sections) if name not in parser]
    if missing:
        sys.exit(f"{path}: missing section(s) {', '.join(missing)}")
    for name, keys in REQUIRED_TEXT.items():
        for key in keys:
            if not parser[name].get(key):
                sys.exit(f"{path}: [{name}] missing {key}")
    for name, limits in REQUIRED_INTS.items():
        for key, (low, high) in limits.items():
            value = parser[name].get(key, "")
            if not (value.isascii() and value.isdigit()
                    and low <= int(value) <= high):
                sys.exit(f"{path}: [{name}] {key} must be a whole number in "
                         f"{low}-{high}, got '{value}'")
    link = parser["gcs.link"]
    if link.getint("live_ms") > link.getint("stale_ms"):
        sys.exit(f"{path}: [gcs.link] live_ms must not exceed stale_ms")
    return parser
