"""The GCS application: owns the state and the network, renders components."""

import tkinter as tk

import state
from components.video_view import VideoView


class App:
    """Polls the services from the Tk event loop and re-renders on change."""

    def __init__(self, root: tk.Tk, settings, video) -> None:
        self._root = root
        self._poll_ms = settings["gcs.link"].getint("poll_ms")
        self._video = video
        self.state = state.GcsState()
        root.title("Ground Control Station")
        self._video_view = VideoView(root)
        self._video_view.grid(row=0, column=0)
        root.bind("<KeyPress-q>", lambda _: root.destroy())
        self._poll()

    def _poll(self) -> None:
        """Shows the newest completed frame.

        Reschedules first, so an unexpected error in one poll cannot stop
        the loop (Tk prints it and the next poll still runs).
        """
        self._root.after(self._poll_ms, self._poll)
        frames = self._video.poll()
        for frame in frames:
            self.state = state.record_frame(self.state, frame.frame_id)
        if frames:
            self._video_view.render(frames[-1].image)
