"""Sizes, colours and layout of the GCS window."""

from constants import text

PAD_PX = 8
# Fits the longest value, "fwd -1.00 right -1.00", so text changes never
# resize the window.
VALUE_WIDTH_CHARS = 21
BUTTON_WIDTH_CHARS = 3
SCALE_LENGTH_PX = 240
GPS_DECIMALS = 5
FRAME_FORMAT = ".ppm"  # Tk reads PPM natively, so no Pillow is needed.

VIDEO_BACKGROUND = "black"
BADGE_TEXT_COLOR = "white"
BADGE_COLORS = {
    text.LINK_NO_DATA: "gray40",
    text.LINK_LIVE: "forest green",
    text.LINK_STALE: "dark orange",
    text.LINK_LOST: "red3",
}
ON_COLOR = "gray60"  # A latched arrow; the others keep the default look.

# Video overlays, by their name in drone.ini [gcs.ui] overlays.
OVERLAY_CROSSHAIR = "crosshair"
OVERLAY_NAMES = (OVERLAY_CROSSHAIR,)
# Crosshair: four arms around the centre with an open gap, so a car under
# the drone stays visible; white with a dark outline to show on any colour.
CROSSHAIR_GAP_PX = 5
CROSSHAIR_ARM_PX = 11
CROSSHAIR_WIDTH_PX = 2
CROSSHAIR_OUTLINE_PX = 4
CROSSHAIR_COLOR = "white"
CROSSHAIR_OUTLINE_COLOR = "black"

# Window grid: (row, column) of each component; the strip spans both columns.
VIDEO_CELL = (0, 0)
PANEL_CELL = (0, 1)
STRIP_ROW = 1
WINDOW_COLUMNS = 2
# Arrow pad: (row, column) of each button in the cross; "stop" is the centre.
PAD_CELLS = {"forward": (0, 1), "left": (1, 0), text.STOP_BUTTON: (1, 1),
             "right": (1, 2), "back": (2, 1)}
