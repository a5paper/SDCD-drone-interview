"""The GCS application: owns the state and the network, renders components."""

import dataclasses
import time
import tkinter as tk

import state
from units import MS_PER_S
from components.telemetry_panel import TelemetryPanel
from components.video_view import VideoView


class App:
    """Polls the services from the Tk event loop and re-renders on change."""

    def __init__(self, root: tk.Tk, settings, video, telemetry) -> None:
        self._root = root
        link = settings["gcs.link"]
        self._poll_ms = link.getint("poll_ms")
        self._live_ms = link.getint("live_ms")
        self._stale_ms = link.getint("stale_ms")
        self._video = video
        self._telemetry = telemetry
        self._last_frame = None
        self._last_frame_s = None
        self.state = state.GcsState()
        root.title("Ground Control Station")
        self._video_view = VideoView(root)
        self._video_view.grid(row=0, column=0)
        self._panel = TelemetryPanel(root)
        self._panel.grid(row=0, column=1, sticky="ns")
        root.bind("<KeyPress-q>", lambda _: root.destroy())
        self._panel.render(self.state)
        self._poll()

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
        if new != self.state:
            self.state = new
            self._panel.render(new)

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
