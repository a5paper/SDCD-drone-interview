"""Side panel: link badge, drone telemetry and stream health."""

import tkinter as tk
from tkinter import ttk

import state as gcs_state
from constants import protocol, text, ui
from constants.protocol import GPS_FIX, GPS_NO_FIX
from constants.text import NONE_TEXT


def percent_text(part: int, whole: int) -> str:
    """Returns '93.7 % of 980', or '-' when whole is 0.

    Only the total is shown, so a 7-digit count (about 4 days of video) still
    fits the value width.
    """
    if whole == 0:
        return NONE_TEXT
    percent = protocol.MAX_PERCENT * part / whole
    return f"{percent:.1f} % of {whole}"


def gps_text(telemetry: gcs_state.TelemetryView) -> str:
    if telemetry.gps_state == GPS_FIX:
        digits = ui.GPS_DECIMALS
        latitude, longitude = telemetry.gps_fix
        return f"{latitude:.{digits}f}, {longitude:.{digits}f}"
    if telemetry.gps_state == GPS_NO_FIX:
        return text.GPS_NO_FIX_TEXT
    return text.NONE_TEXT


class TelemetryPanel(tk.Frame):
    """Shows the state as text. Holds no state of its own."""

    def __init__(self, parent) -> None:
        super().__init__(parent, padx=ui.PAD_PX, pady=ui.PAD_PX)
        self._badge = tk.Label(self, foreground=ui.BADGE_TEXT_COLOR,
                               pady=ui.PAD_PX)
        self._badge.grid(row=0, column=0, columnspan=2, sticky="ew")
        self._values = {}
        for row, name in enumerate(text.ROWS, start=1):
            tk.Label(self, text=name, anchor="w").grid(row=row, column=0,
                                                       sticky="w")
            value = tk.Label(self, anchor="e", width=ui.VALUE_WIDTH_CHARS)
            value.grid(row=row, column=1, sticky="e")
            self._values[name] = value
        self._battery = ttk.Progressbar(self, maximum=protocol.MAX_PERCENT)
        self._battery.grid(row=len(text.ROWS) + 1, column=0, columnspan=2,
                           sticky="ew", pady=ui.PAD_PX)

    def render(self, state: gcs_state.GcsState) -> None:
        """Updates every label that differs from state."""
        _set(self._badge, text=state.link,
             background=ui.BADGE_COLORS[state.link])
        telemetry = state.telemetry
        texts = dict.fromkeys(text.ROWS, NONE_TEXT)
        battery = 0.0
        if telemetry is not None:
            forward, right = telemetry.last_command
            battery = telemetry.battery_percentage
            texts[text.ROW_SPEED] = f"{telemetry.speed_mps:.1f} m/s"
            texts[text.ROW_BATTERY] = f"{battery:.1f} %"
            texts[text.ROW_GPS] = gps_text(telemetry)
            command = f"fwd {forward:+.2f} right {right:+.2f}"
            texts[text.ROW_LAST_COMMAND] = command
        if state.telemetry_gap_ms is not None:
            texts[text.ROW_GAP] = f"{state.telemetry_gap_ms:.0f} ms"
        matches = gcs_state.command_matches(state)
        texts[text.ROW_MATCHES] = text.MATCH_TEXT[matches]
        texts[text.ROW_FRAMES] = percent_text(state.shown, state.sent)
        texts[text.ROW_REBUILT] = str(state.recovered)
        lost = percent_text(state.telemetry_lost, state.telemetry_expected)
        texts[text.ROW_TELEMETRY_LOST] = lost
        for name, value in texts.items():
            _set(self._values[name], text=value)
        if self._battery["value"] != battery:
            self._battery["value"] = battery


def _set(widget, **options) -> None:
    """Configures only the options that changed, to avoid redraws."""
    changed = {key: value for key, value in options.items()
               if widget.cget(key) != value}
    if changed:
        widget.configure(**changed)
