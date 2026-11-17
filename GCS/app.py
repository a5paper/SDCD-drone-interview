"""The GCS application: owns the state and the network, renders components."""

import dataclasses
import time
import tkinter as tk

import config
import state
from components.command_strip import CommandStrip
from components.overlays.crosshair import Crosshair
from components.telemetry_panel import TelemetryPanel
from components.video_view import VideoView
from constants import keys, text, ui
from constants import settings as settings_names
from constants.protocol import MS_PER_S
from services.command_sender import next_interval

# Overlay name in drone.ini to its component (see VideoView).
OVERLAYS = {ui.OVERLAY_CROSSHAIR: Crosshair}


class App:
    """Polls the services from the Tk event loop and re-renders on change."""

    def __init__(self, root: tk.Tk, settings, video, telemetry,
                 sender) -> None:
        self._root = root
        link = settings[settings_names.LINK]
        commands = settings[settings_names.COMMANDS]
        ui_settings = settings[settings_names.UI]
        self._poll_ms = link.getint(settings_names.POLL_MS)
        self._live_ms = link.getint(settings_names.LIVE_MS)
        self._stale_ms = link.getint(settings_names.STALE_MS)
        self._resend_ms = commands.getint(settings_names.RESEND_MS)
        self._backoff_max_ms = commands.getint(settings_names.BACKOFF_MAX_MS)
        drag_key = settings_names.SPEED_DRAG_STEP_PERCENT
        nudge_key = settings_names.SPEED_NUDGE_STEP_PERCENT
        self._drag_step = ui_settings.getint(drag_key)
        self._nudge_step = ui_settings.getint(nudge_key)
        self._video = video
        self._telemetry = telemetry
        self._sender = sender
        self._last_frame = None
        self._last_frame_s = None
        self._send_interval_ms = self._resend_ms
        self._send_job = None
        start = ui_settings.getint(settings_names.SPEED_START_PERCENT)
        self.state = state.GcsState(speed_percent=start)

        root.title(text.WINDOW_TITLE)
        overlays = [OVERLAYS[name] for name in config.overlays(settings)]
        self._video_view = VideoView(root, overlays)
        self._video_view.grid(row=ui.VIDEO_CELL[0], column=ui.VIDEO_CELL[1])
        self._panel = TelemetryPanel(root)
        self._panel.grid(row=ui.PANEL_CELL[0], column=ui.PANEL_CELL[1],
                         sticky="ns")
        self._strip = CommandStrip(
            root, self._drag_step, self._nudge_step, on_arrow=self._click_arrow,
            on_stop=self._stop, on_speed=self._speed, on_nudge=self._nudge)
        self._strip.grid(row=ui.STRIP_ROW, column=0,
                         columnspan=ui.WINDOW_COLUMNS, sticky="ew")
        root.bind("<KeyPress>", self._key)
        self._panel.render(self.state)
        self._strip.render(self.state)
        self._poll()
        self._send()

    def _apply(self, new: state.GcsState) -> None:
        """Makes new the current state and re-renders if anything changed.

        Leaving the LOST link state resends at once, so the drone gets the
        current command without waiting out a backoff interval.
        """
        if new == self.state:
            return
        recovered = (self.state.link == text.LINK_LOST and
                     new.link != text.LINK_LOST)
        self.state = new
        self._panel.render(new)
        self._strip.render(new)
        self._video_view.render_state(new)
        if recovered:
            self._send()

    # User input: each changes the command, so it is sent at once.

    def _arrow(self, arrow: str) -> None:
        self._input(state.press_arrow(self.state, arrow))

    def _click_arrow(self, arrow: str) -> None:
        self._input(state.click_arrow(self.state, arrow))

    def _stop(self) -> None:
        self._input(state.stop(self.state))

    def _speed(self, percent: int) -> None:
        # A drag reports every mouse motion; only a new value is an input.
        if percent != self.state.speed_percent:
            self._input(state.set_speed(self.state, percent))

    def _nudge(self, delta: int) -> None:
        percent = self.state.speed_percent + delta
        self._input(state.set_speed(self.state, percent))

    def _input(self, new: state.GcsState) -> None:
        self._apply(new)
        self._send(pressed=True)

    def _key(self, event) -> None:
        """Maps a key press to the same handlers the buttons use."""
        key = event.keysym.lower()
        if key in keys.ARROW_KEYS:
            self._arrow(keys.ARROW_KEYS[key])
        elif key == keys.STOP_KEY:
            self._stop()
        elif key in keys.NUDGE_KEYS:
            sign, big = keys.NUDGE_KEYS[key]
            big = big or bool(event.state & keys.SHIFT_MASK)
            step = self._drag_step if big else self._nudge_step
            self._nudge(sign * step)
        elif key == keys.QUIT_KEY:
            self._root.destroy()

    def _send(self, pressed: bool = False) -> None:
        """Sends the current command and schedules the next resend.

        Schedules first, so an error while sending or rendering cannot stop
        the resends (the drone would keep its last command forever).
        """
        if self._send_job is not None:
            self._root.after_cancel(self._send_job)
        link_lost = self.state.link == text.LINK_LOST
        self._send_interval_ms = next_interval(
            self._send_interval_ms, link_lost, pressed, self._resend_ms,
            self._backoff_max_ms)
        self._send_job = self._root.after(self._send_interval_ms, self._send)
        command = state.command(self.state)
        self._sender.send(command)
        self._apply(dataclasses.replace(self.state, sent_command=command))

    def _poll(self) -> None:
        """Reads both streams, updates the state, renders what changed.

        Reschedules first, so an unexpected error in one poll cannot stop
        the loop (Tk prints it and the next poll still runs).
        """
        self._root.after(self._poll_ms, self._poll)
        self._telemetry.poll()
        frames = self._video.poll()
        now_s = time.monotonic()
        new = self.state
        for frame in frames:
            new = state.record_frame(new, frame.frame_id)
        if frames:
            self._last_frame, self._last_frame_s = frames[-1], now_s
            self._video_view.render(self._last_frame.image)
        new = dataclasses.replace(new, **self._telemetry_fields(now_s))
        self._apply(new)

    def _telemetry_fields(self, now_s: float) -> dict:
        """Returns the telemetry part of the state for this poll.

        While video is live the panel shows the telemetry sent with the shown
        frame; once video is stale it shows the newest packet, so telemetry
        keeps updating when only video is lost.
        """
        tracker = self._telemetry.tracker
        arrival_s = self._telemetry.last_arrival_s
        since_ms = None if arrival_s is None else (now_s - arrival_s) * MS_PER_S
        fields = {"link": state.link_status(since_ms, self._live_ms,
                                            self._stale_ms),
                  "recovered": self._video.assembler.recovered,
                  "telemetry_lost": tracker.lost,
                  "telemetry_expected": tracker.expected,
                  "telemetry": None,
                  "telemetry_gap_ms": None}
        newest = self._telemetry.newest
        if newest is not None:
            fields["telemetry"] = state.view_of(newest)
        frame = self._last_frame
        if frame is None:
            return fields
        video_ms = (now_s - self._last_frame_s) * MS_PER_S
        matched = self._telemetry.for_frame(frame.frame_id)
        if video_ms > self._live_ms or matched is None:
            return fields
        gap_s = abs(frame.timestamp_s - matched.timestamp_s)
        fields.update(telemetry=state.view_of(matched),
                      telemetry_gap_ms=gap_s * MS_PER_S)
        return fields
