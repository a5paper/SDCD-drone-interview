"""Sends velocity commands to the simulator, with resend and backoff timing."""

import socket
import time

from constants.protocol import BACKOFF_FACTOR, NS_PER_MS
from protocols import commands_pb2


def next_interval(interval_ms: int, link_lost: bool, pressed: bool,
                  resend_ms: int, backoff_max_ms: int) -> int:
    """Returns the wait before the next resend; the sender's state machine.

    The state is the current interval: resend_ms is STEADY, anything else is
    BACKOFF(interval). Called after every send.

    | State      | send, link ok | send, link LOST   | user input       |
    |------------|---------------|-------------------|------------------|
    | STEADY     | STEADY        | BACKOFF(2 resend) | STEADY           |
    | BACKOFF(i) | STEADY        | BACKOFF(2i <= max)| BACKOFF(resend)* |

    * A press while LOST restarts the backoff at resend_ms; it doubles again
    on the next sends. The App also resends at once when the link leaves
    LOST, which returns it to STEADY.

    A pure function, so every cell is a one-line test. The command is an
    absolute velocity, so a resend is harmless and replaces a lost packet;
    backing off while nobody answers keeps a dead link quiet.
    """
    if not link_lost or pressed:
        return resend_ms
    return min(interval_ms * BACKOFF_FACTOR, backoff_max_ms)


class CommandSender:
    """A connected UDP socket that sends VelocityChangeCommand messages.

    Send errors are counted, not raised: on a connected socket a send to a
    port nobody listens on fails with ECONNREFUSED (from the ICMP reply), which
    is normal while the simulator is not running. On Windows such a send
    never fails, so "0 failed" is normal there.
    """

    def __init__(self, address: str, port: int) -> None:
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.connect((address, port))
        self._sock.setblocking(False)
        self.sent = 0
        self.failed = 0

    def send(self, command: tuple) -> None:
        """Sends (forward, right) as one datagram."""
        forward, right = command
        timestamp_ms = time.time_ns() // NS_PER_MS
        message = commands_pb2.VelocityChangeCommand(
            velocity_x=forward, velocity_y=right, timestamp_ms=timestamp_ms)
        try:
            self._sock.send(message.SerializeToString())
            self.sent += 1
        except OSError:
            self.failed += 1
