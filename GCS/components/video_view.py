"""Shows the newest video frame."""

import tkinter as tk

import cv2

from constants import ui


class VideoView(tk.Label):
    """A label that displays an OpenCV image.

    Tk reads PPM natively, so frames go through cv2.imencode(".ppm") instead of
    Pillow. One PhotoImage is reused; Tk keeps no reference of its own, so it
    is stored on self.
    """

    def __init__(self, parent) -> None:
        super().__init__(parent, background=ui.VIDEO_BACKGROUND)
        self._photo = tk.PhotoImage()
        self.configure(image=self._photo)

    def render(self, image) -> None:
        """Shows image, a BGR array."""
        ok, ppm = cv2.imencode(ui.FRAME_FORMAT, image)
        if ok:
            self._photo.configure(data=ppm.tobytes())
