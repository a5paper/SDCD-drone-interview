"""Loads config/drone.ini, the settings shared with the simulator.

Every setting is checked once at startup; a bad file exits with a message
instead of failing later in the middle of a run.
"""

import configparser
import pathlib
import sys

from constants import settings as names
from constants.protocol import MAX_PERCENT

DEFAULT_PATH = (pathlib.Path(__file__).resolve().parents[1] / "config" /
                "drone.ini")
PORT = (1, 65535)  # Valid UDP ports.
POSITIVE = (1, sys.maxsize)
PERCENT = (0, MAX_PERCENT)
STEP_PERCENT = (1, MAX_PERCENT)
REQUIRED_TEXT = {
    names.NETWORK: (names.SIMULATOR_ADDRESS, names.GCS_ADDRESS),
    names.LINK: (names.BIND_ADDRESS,),
}
# Each key must be a whole number within its (low, high) range.
REQUIRED_INTS = {
    names.NETWORK: {names.COMMAND_PORT: PORT, names.TELEMETRY_PORT: PORT,
                    names.VIDEO_PORT: PORT},
    names.LINK: {names.POLL_MS: POSITIVE, names.RESTART_GAP_FRAMES: POSITIVE,
                 names.RESTART_GAP_PACKETS: POSITIVE,
                 names.TELEMETRY_KEPT: POSITIVE, names.LIVE_MS: POSITIVE,
                 names.STALE_MS: POSITIVE},
    names.COMMANDS: {names.RESEND_MS: POSITIVE,
                     names.BACKOFF_MAX_MS: POSITIVE,
                     names.EXIT_STOP_REPEATS: POSITIVE},
    names.UI: {names.SPEED_START_PERCENT: PERCENT,
               names.SPEED_DRAG_STEP_PERCENT: STEP_PERCENT,
               names.SPEED_NUDGE_STEP_PERCENT: STEP_PERCENT},
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
                allowed = f"at least {low}"
                if high < sys.maxsize:
                    allowed = f"{low}-{high}"
                sys.exit(f"{path}: [{name}] {key} must be a whole number, "
                         f"{allowed}, got '{value}'")
    link = parser[names.LINK]
    if link.getint(names.LIVE_MS) > link.getint(names.STALE_MS):
        sys.exit(f"{path}: [{names.LINK}] {names.LIVE_MS} must not exceed "
                 f"{names.STALE_MS}")
    return parser
