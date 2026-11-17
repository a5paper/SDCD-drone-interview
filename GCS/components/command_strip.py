"""Bottom strip: latching arrow pad and the intended speed slider."""

import tkinter as tk

import state as gcs_state
from constants import protocol, text, ui


class CommandStrip(tk.Frame):
    """Reports clicks through callbacks; shows the latched state it is given.

    No widget takes keyboard focus, so keys always reach the window's
    bindings (a focused Scale would also move itself on Left and Right).
    """

    def __init__(self, parent, drag_step: int, nudge_step: int, on_arrow,
                 on_stop, on_speed, on_nudge) -> None:
        super().__init__(parent, padx=ui.PAD_PX, pady=ui.PAD_PX)
        self._on_speed = on_speed
        self._shown_speed = None
        self._arrows = {}
        pad = tk.Frame(self)
        pad.grid(row=0, column=0, rowspan=2, padx=ui.PAD_PX)
        for arrow, (row, column) in ui.PAD_CELLS.items():
            stop = arrow == text.STOP_BUTTON
            action = on_stop if stop else _bind(on_arrow, arrow)
            button = _button(pad, text.GLYPHS[arrow], action)
            button.grid(row=row, column=column)
            self._arrows[arrow] = button
        del self._arrows[text.STOP_BUTTON]
        self._default_color = button.cget("background")

        tk.Label(self, text=text.SPEED_LABEL).grid(row=0, column=1)
        speed = tk.Frame(self)
        speed.grid(row=1, column=1)
        down = _button(speed, text.NUDGE_DOWN, _bind(on_nudge, -nudge_step))
        down.grid(row=0, column=0)
        # The resolution makes Tk itself keep the knob on drag-step notches,
        # so the GCS never moves the knob away from the mouse (that fight
        # made dragging jitter). A 1 % nudge (53 %) leaves the knob on the
        # nearest notch; the label shows the exact value.
        self._scale = tk.Scale(speed, from_=0, to=protocol.MAX_PERCENT,
                               resolution=drag_step, orient=tk.HORIZONTAL,
                               showvalue=False, takefocus=0,
                               length=ui.SCALE_LENGTH_PX)
        self._scale.grid(row=0, column=1)
        # Read the knob after Tk's own binding has moved it.
        for event in ("<B1-Motion>", "<ButtonRelease-1>"):
            self._scale.bind(event, lambda _: self.after_idle(self._dragged))
        up = _button(speed, text.NUDGE_UP, _bind(on_nudge, nudge_step))
        up.grid(row=0, column=2)
        widest = _percent_text(protocol.MAX_PERCENT)
        self._percent = tk.Label(speed, width=len(widest))
        self._percent.grid(row=0, column=3)

    def _dragged(self) -> None:
        """Reports the knob's notch (already a multiple of the drag step)."""
        self._on_speed(int(self._scale.get()))

    def render(self, state: gcs_state.GcsState) -> None:
        """Shows the latched arrows and the intended speed."""
        for arrow, button in self._arrows.items():
            axis, sign = gcs_state.ARROWS[arrow]
            on = state.direction[axis] == sign
            relief = tk.SUNKEN if on else tk.RAISED
            if button.cget("relief") != relief:
                color = ui.ON_COLOR if on else self._default_color
                button.configure(relief=relief, background=color)
        if state.speed_percent != self._shown_speed:
            self._shown_speed = state.speed_percent
            self._scale.set(state.speed_percent)  # Tk puts it on a notch.
        label = _percent_text(state.speed_percent)
        if self._percent.cget("text") != label:
            self._percent.configure(text=label)


def _bind(callback, argument):
    """Returns a no-argument function that calls callback(argument)."""
    return lambda: callback(argument)


def _button(parent, label: str, action) -> tk.Button:
    """Returns a small button that never takes keyboard focus."""
    return tk.Button(parent, text=label, width=ui.BUTTON_WIDTH_CHARS,
                     takefocus=0, command=action)


def _percent_text(percent: int) -> str:
    """Returns the speed label, e.g. '53 %'."""
    return f"{percent} %"
