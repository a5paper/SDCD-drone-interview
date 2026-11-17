"""Keyboard map (Tk keysyms, lower case). Keys call the buttons' handlers."""

ARROW_KEYS = {"w": "forward", "up": "forward", "s": "back", "down": "back",
              "a": "left", "left": "left", "d": "right", "right": "right"}
STOP_KEY = "space"
QUIT_KEY = "q"
# Key to (direction, uses the big step). The big step is the same keys with
# Shift: "_" and "+" on the main keys, Shift on the keypad.
NUDGE_KEYS = {"minus": (-1, False), "equal": (1, False),
              "underscore": (-1, True), "plus": (1, True),
              "kp_subtract": (-1, False), "kp_add": (1, False)}
SHIFT_MASK = 0x1  # Bit of a Tk key event's state that is set by Shift.
