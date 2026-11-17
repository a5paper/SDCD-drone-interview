"""The GCS state and the pure functions that change it. No tkinter here."""

import dataclasses
from typing import Optional


@dataclasses.dataclass(frozen=True)
class GcsState:
    """Everything the components render. Only App replaces it."""

    shown: int = 0
    # Frames sent in the simulator runs before the current one (restarts).
    sent_before: int = 0
    first_frame_id: Optional[int] = None
    last_frame_id: Optional[int] = None

    @property
    def sent(self) -> int:
        """Frames the simulator sent since the first one shown."""
        if self.first_frame_id is None:
            return 0
        return (self.sent_before + self.last_frame_id - self.first_frame_id
                + 1)


def record_frame(state: GcsState, frame_id: int) -> GcsState:
    """Returns state with one more frame shown.

    A frame_id below the last one means the simulator restarted (the
    assembler only lets that through after a restart), so a new run starts.
    """
    if state.last_frame_id is not None and frame_id < state.last_frame_id:
        state = dataclasses.replace(state, sent_before=state.sent,
                                    first_frame_id=None)
    first = frame_id if state.first_frame_id is None else state.first_frame_id
    return dataclasses.replace(state, shown=state.shown + 1,
                               first_frame_id=first, last_frame_id=frame_id)
