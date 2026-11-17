"""Crosshair at the centre of the video: where the drone is."""

from constants import ui

# Unit direction of each arm from the centre: up, down, left, right.
_ARMS = ((0, -1), (0, 1), (-1, 0), (1, 0))
# Outline first, so the white line is drawn on top of it.
_STYLES = ((ui.CROSSHAIR_OUTLINE_PX, ui.CROSSHAIR_OUTLINE_COLOR),
           (ui.CROSSHAIR_WIDTH_PX, ui.CROSSHAIR_COLOR))


class Crosshair:
    """Four short arms around an open centre; it does not change with state.

    The overlay contract (see VideoView): __init__(canvas) keeps the canvas,
    place(width, height) draws or moves the items when the frame size is
    known, render(state) updates them on every state change.
    """

    def __init__(self, canvas) -> None:
        self._canvas = canvas
        self._items = []  # (item, dx, dy) for every arm of every style.

    def place(self, width: int, height: int) -> None:
        """Draws the arms around the centre, or moves them there."""
        if not self._items:
            for line_width, color in _STYLES:
                for dx, dy in _ARMS:
                    item = self._canvas.create_line(0, 0, 0, 0, fill=color,
                                                    width=line_width)
                    self._items.append((item, dx, dy))
        center_x, center_y = width / 2, height / 2
        gap, end = ui.CROSSHAIR_GAP_PX, ui.CROSSHAIR_ARM_PX
        for item, dx, dy in self._items:
            self._canvas.coords(item, center_x + dx * gap, center_y + dy * gap,
                                center_x + dx * end, center_y + dy * end)

    def render(self, state) -> None:
        """Static: nothing here depends on the state."""
