"""Shows the newest video frame with optional overlays on top."""

import tkinter as tk

import cv2

from constants import ui


class VideoView(tk.Canvas):
    """A canvas with the frame image at the bottom and overlays above it.

    Tk reads PPM natively, so frames go through cv2.imencode(".ppm") instead of
    Pillow. One PhotoImage is reused; Tk keeps no reference of its own, so it
    is stored on self.

    Overlays are separate components (components/overlays/), chosen in
    drone.ini [gcs.ui] overlays. Each is built with this canvas and has
    place(width, height), called when the frame size is known or changes,
    and render(state), called on every state change. A new overlay (a
    battery gauge, a heading arrow) is one more class; nothing here changes.
    """

    def __init__(self, parent, overlay_classes) -> None:
        super().__init__(parent, background=ui.VIDEO_BACKGROUND,
                         highlightthickness=0, width=0, height=0)
        self._photo = tk.PhotoImage()
        self.create_image(0, 0, anchor=tk.NW, image=self._photo)
        # Created after the image, so their items are drawn above it.
        self._overlays = [overlay(self) for overlay in overlay_classes]
        self._size = None

    def render(self, image) -> None:
        """Shows image, a BGR array; places the overlays on a new size."""
        ok, ppm = cv2.imencode(ui.FRAME_FORMAT, image)
        if not ok:
            return
        self._photo.configure(data=ppm.tobytes())
        height, width = image.shape[:2]
        if (width, height) != self._size:
            self._size = (width, height)
            self.configure(width=width, height=height)
            for overlay in self._overlays:
                overlay.place(width, height)

    def render_state(self, state) -> None:
        """Lets every overlay show the current state."""
        for overlay in self._overlays:
            overlay.render(state)
