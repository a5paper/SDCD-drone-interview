"""User-visible strings used in more than one place."""

WINDOW_TITLE = "Ground Control Station"

# Link badge states (also the badge text).
LINK_NO_DATA = "NO DATA"
LINK_LIVE = "LIVE"
LINK_STALE = "STALE"
LINK_LOST = "LOST"

NONE_TEXT = "-"  # A value that is not known yet.
MATCH_TEXT = {None: NONE_TEXT, True: "yes", False: "no"}
GPS_NO_FIX_TEXT = "no fix"

# Telemetry panel rows, top to bottom.
ROW_SPEED = "Speed"
ROW_BATTERY = "Battery"
ROW_GPS = "GPS"
ROW_LAST_COMMAND = "Last command"
ROW_MATCHES = "Matches sent"
ROW_GAP = "Video/telemetry gap"
ROW_FRAMES = "Frames shown"
ROW_REBUILT = "Rebuilt by FEC"
ROW_TELEMETRY_LOST = "Telemetry lost"
ROWS = (ROW_SPEED, ROW_BATTERY, ROW_GPS, ROW_LAST_COMMAND, ROW_MATCHES, ROW_GAP,
        ROW_FRAMES, ROW_REBUILT, ROW_TELEMETRY_LOST)

# Command strip.
SPEED_LABEL = "Intended speed"
STOP_BUTTON = "stop"  # Name of the centre pad button (not an arrow).
GLYPHS = {"forward": "▲", "left": "◀", STOP_BUTTON: "■", "right": "▶",
          "back": "▼"}
NUDGE_DOWN = "−"
NUDGE_UP = "+"
