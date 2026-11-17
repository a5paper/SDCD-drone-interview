"""The GCS state and the pure functions that change it. No tkinter here."""

import dataclasses
from typing import Optional

LINK_NO_DATA = "NO DATA"
LINK_LIVE = "LIVE"
LINK_STALE = "STALE"
LINK_LOST = "LOST"

# A velocity command as plain values: (forward, right), each in [-1, 1].
Command = tuple


@dataclasses.dataclass(frozen=True)
class TelemetryView:
    """The telemetry fields the GCS shows, copied out of the protobuf message.

    Plain immutable values, so the frozen state really is immutable (a
    protobuf message inside it could still be changed).
    """

    speed_mps: float
    battery_percentage: float
    last_command: Command


def view_of(telemetry) -> TelemetryView:
    """Copies the shown fields out of a Telemetry message."""
    command = telemetry.last_command
    return TelemetryView(telemetry.speed_mps, telemetry.battery_percentage,
                         (command.velocity_x, command.velocity_y))


@dataclasses.dataclass(frozen=True)
class GcsState:
    """Everything the components render. Only App replaces it."""

    shown: int = 0
    # Frames sent in the simulator runs before the current one (restarts).
    sent_before: int = 0
    first_frame_id: Optional[int] = None
    last_frame_id: Optional[int] = None
    recovered: int = 0
    link: str = LINK_NO_DATA
    # Telemetry for the shown frame (the newest one while video is stale).
    telemetry: Optional[TelemetryView] = None
    # Time between the shown frame and its telemetry, either way round (an
    # exact match is sent up to one frame later); None without live video.
    telemetry_gap_ms: Optional[float] = None
    telemetry_lost: int = 0
    telemetry_expected: int = 0
    # The last command the GCS sent (set by the command strip, C.3).
    sent_command: Optional[Command] = None

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


def link_status(since_ms: Optional[float], live_ms: int,
                stale_ms: int) -> str:
    """Returns the link badge for the time since the last telemetry packet.

    A pure function of that time: the badge needs no memory, so it is not a
    state machine.
    """
    if since_ms is None:
        return LINK_NO_DATA
    if since_ms <= live_ms:
        return LINK_LIVE
    if since_ms <= stale_ms:
        return LINK_STALE
    return LINK_LOST


def command_matches(state: GcsState) -> Optional[bool]:
    """Whether the drone's last command is the one sent; None if unknown.

    Exact float comparison is right here: the drone stores the command it
    received unchanged and echoes those same values back.
    """
    if state.telemetry is None or state.sent_command is None:
        return None
    return state.telemetry.last_command == state.sent_command
