"""Side panel: link badge, drone telemetry and stream health."""

import tkinter as tk
from tkinter import ttk

import state as gcs_state

# Fits the longest value, "fwd -1.00 right -1.00", so text changes never
# resize the window.
VALUE_WIDTH_CHARS = 21
PAD_PX = 8
NONE_TEXT = "-"
BADGE_COLORS = {  # Background per link status; text is white.
    gcs_state.LINK_NO_DATA: "gray40",
    gcs_state.LINK_LIVE: "forest green",
    gcs_state.LINK_STALE: "dark orange",
    gcs_state.LINK_LOST: "red3",
}
MATCH_TEXT = {None: NONE_TEXT, True: "yes", False: "no"}
ROWS = ("Speed", "Battery", "Last command", "Matches sent",
        "Video/telemetry gap", "Frames shown", "Rebuilt by FEC",
        "Telemetry lost")


def percent_text(part: int, whole: int) -> str:
    """Returns '93.7 % (918/980)', or '-' when whole is 0."""
    if whole == 0:
        return NONE_TEXT
    return f"{100.0 * part / whole:.1f} % ({part}/{whole})"


class TelemetryPanel(tk.Frame):
    """Shows the state as text. Holds no state of its own."""

    def __init__(self, parent) -> None:
        super().__init__(parent, padx=PAD_PX, pady=PAD_PX)
        self._badge = tk.Label(self, foreground="white", pady=PAD_PX)
        self._badge.grid(row=0, column=0, columnspan=2, sticky="ew")
        self._values = {}
        for row, name in enumerate(ROWS, start=1):
            tk.Label(self, text=name, anchor="w").grid(row=row, column=0,
                                                       sticky="w")
            value = tk.Label(self, anchor="e", width=VALUE_WIDTH_CHARS)
            value.grid(row=row, column=1, sticky="e")
            self._values[name] = value
        self._battery = ttk.Progressbar(self, maximum=100)
        self._battery.grid(row=len(ROWS) + 1, column=0, columnspan=2,
                           sticky="ew", pady=PAD_PX)

    def render(self, state: gcs_state.GcsState) -> None:
        """Updates every label that differs from state."""
        _set(self._badge, text=state.link,
             background=BADGE_COLORS[state.link])
        telemetry = state.telemetry
        texts = dict.fromkeys(ROWS, NONE_TEXT)
        battery = 0.0
        if telemetry is not None:
            forward, right = telemetry.last_command
            battery = telemetry.battery_percentage
            texts["Speed"] = f"{telemetry.speed_mps:.1f} m/s"
            texts["Battery"] = f"{battery:.1f} %"
            texts["Last command"] = f"fwd {forward:+.2f} right {right:+.2f}"
        if state.telemetry_gap_ms is not None:
            texts["Video/telemetry gap"] = f"{state.telemetry_gap_ms:.0f} ms"
        texts["Matches sent"] = MATCH_TEXT[gcs_state.command_matches(state)]
        texts["Frames shown"] = percent_text(state.shown, state.sent)
        texts["Rebuilt by FEC"] = str(state.recovered)
        texts["Telemetry lost"] = percent_text(state.telemetry_lost,
                                               state.telemetry_expected)
        for name, text in texts.items():
            _set(self._values[name], text=text)
        if self._battery["value"] != battery:
            self._battery["value"] = battery


def _set(widget, **options) -> None:
    """Configures only the options that changed, to avoid redraws."""
    changed = {key: value for key, value in options.items()
               if widget.cget(key) != value}
    if changed:
        widget.configure(**changed)
