"""The GCS state and the pure functions that change it. No tkinter here."""

import dataclasses
import math
from typing import Optional

from constants.protocol import (GPS_FIX, GPS_STATE_ONEOF, MAX_PERCENT,
                                STOP_COMMAND)
from constants.text import LINK_LIVE, LINK_LOST, LINK_NO_DATA, LINK_STALE

NO_DIRECTION = (0, 0)  # (forward, right) with no arrow latched.

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
    gps_state: Optional[str] = None
    gps_fix: Optional[tuple] = None


def view_of(telemetry) -> TelemetryView:
    """Copies the shown fields out of a Telemetry message."""
    command = telemetry.last_command
    gps_state = telemetry.gps_status.WhichOneof(GPS_STATE_ONEOF)
    fix = telemetry.gps_status.fix
    gps_fix = (fix.latitude_deg, fix.longitude_deg)
    return TelemetryView(telemetry.speed_mps, telemetry.battery_percentage,
                         (command.velocity_x, command.velocity_y), gps_state,
                         gps_fix if gps_state == GPS_FIX else None)


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
    # The last command the GCS sent to the drone.
    sent_command: Optional[Command] = None
    # Latched direction per axis, (forward, right), each -1, 0 or 1.
    direction: tuple = NO_DIRECTION
    # Intended speed, percent of the drone's maximum.
    speed_percent: int = 0

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


# Arrow name: (axis index in direction, sign). Forward is up on screen.
ARROWS = {
    "forward": (0, 1),
    "back": (0, -1),
    "left": (1, -1),
    "right": (1, 1),
}


def press_arrow(state: GcsState, arrow: str) -> GcsState:
    """Returns state with the arrow's direction latched on its axis.

    The opposite arrow reverses that axis; the other axis is kept, so
    forward then right is a diagonal. Pressing an arrow that is already on
    changes nothing, so a held key that auto-repeats cannot flicker the
    direction. Only stop clears an axis.
    """
    axis, sign = ARROWS[arrow]
    direction = list(state.direction)
    direction[axis] = sign
    return dataclasses.replace(state, direction=tuple(direction))


def click_arrow(state: GcsState, arrow: str) -> GcsState:
    """Returns state after a mouse click on an arrow button.

    Clicking a latched (grayed) arrow clears its axis; any other click acts
    like press_arrow. Only clicks toggle: a click cannot auto-repeat, a held
    key can.
    """
    axis, sign = ARROWS[arrow]
    if state.direction[axis] != sign:
        return press_arrow(state, arrow)
    direction = list(state.direction)
    direction[axis] = 0
    return dataclasses.replace(state, direction=tuple(direction))


def stop(state: GcsState) -> GcsState:
    """Returns state with no direction latched."""
    return dataclasses.replace(state, direction=NO_DIRECTION)


def set_speed(state: GcsState, percent: int) -> GcsState:
    """Returns state with the intended speed clamped to 0-100 %."""
    clamped = min(max(percent, 0), MAX_PERCENT)
    return dataclasses.replace(state, speed_percent=clamped)


def command(state: GcsState) -> Command:
    """Returns the velocity to send: unit direction times intended speed.

    A diagonal is normalized, so it is no faster than a straight line (the
    simulator caps the command length at 1 the same way).
    """
    forward, right = state.direction
    length = math.hypot(forward, right)
    if length == 0:
        return STOP_COMMAND
    scale = state.speed_percent / MAX_PERCENT / length
    return (forward * scale, right * scale)
