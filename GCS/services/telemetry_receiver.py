"""Receives the simulator's telemetry and keeps recent packets by frame_id."""

import collections
import time

from google.protobuf import message

from protocols import telemetry_pb2
from services import udp
from constants.protocol import MS_PER_S


class SequenceTracker:
    """Counts lost packets from sequence numbers, across simulator restarts.

    Only counters and the last `window` sequence numbers are kept (not every
    number seen), so memory stays constant however long the GCS runs. A number
    still in the window is a duplicate and is not counted as received. A lower
    number after silence, or one more than restart_gap below the highest,
    starts a new run (counted in restarts); earlier runs stay in the totals.
    """

    def __init__(self, restart_gap: int, window: int) -> None:
        self._restart_gap = restart_gap
        self._recent = collections.deque(maxlen=window)
        self._first = None
        self._highest = None
        self._received = 0
        self._expected_before = 0
        self._received_before = 0
        self.duplicates = 0
        self.restarts = 0

    def add(self, sequence: int, after_silence: bool) -> bool:
        """Records one received sequence number.

        after_silence: the stream was silent before this packet (link LOST).
        A lower number then is a restart, since a late packet does not arrive
        after that long; without silence only a drop of more than restart_gap
        is. Returns True when it is the newest packet so far (the first, a new
        highest, or the first of a new run), False for late and duplicate
        packets.
        """
        first_packet = self._highest is None
        dropped = not first_packet and sequence < self._highest
        restarted = dropped and (after_silence or
                                 sequence < self._highest - self._restart_gap)
        self.restarts += restarted
        if first_packet or restarted:
            self._expected_before = self.expected
            self._received_before += self._received
            self._first = self._highest = sequence
            self._received = 0
            self._recent.clear()
        if sequence in self._recent:
            self.duplicates += 1
            return False
        newest = sequence >= self._highest
        self._recent.append(sequence)
        self._received += 1
        self._first = min(self._first, sequence)
        self._highest = max(self._highest, sequence)
        return newest

    @property
    def expected(self) -> int:
        """Packets the simulator sent, from the first to the newest seen."""
        if self._highest is None:
            return self._expected_before
        return self._expected_before + self._highest - self._first + 1

    @property
    def lost(self) -> int:
        """Packets expected but not received."""
        received = self._received_before + self._received
        return max(0, self.expected - received)


class TelemetryReceiver:
    """Listens on the telemetry port; finds the packet that matches a frame.

    Keeps the last `kept` packets keyed by the frame_id they were sent after.
    A datagram that is not a valid Telemetry message is counted in rejected.
    """

    def __init__(self, address: str, port: int, kept: int, restart_gap: int,
                 silence_ms: int) -> None:
        self._sock = udp.open_receiver(address, port)
        self._kept = kept
        self._silence_s = silence_ms / MS_PER_S
        self._by_frame = collections.OrderedDict()
        self.tracker = SequenceTracker(restart_gap, kept)
        self.newest = None
        self.last_arrival_s = None
        self.rejected = 0

    def poll(self) -> None:
        """Reads every waiting packet."""
        for datagram in udp.drain(self._sock):
            telemetry = telemetry_pb2.Telemetry()
            try:
                telemetry.ParseFromString(datagram)
            except message.DecodeError:
                self.rejected += 1
                continue
            now_s = time.monotonic()
            after_silence = (self.last_arrival_s is not None and
                             now_s - self.last_arrival_s > self._silence_s)
            self.last_arrival_s = now_s
            restarts = self.tracker.restarts
            if not self.tracker.add(telemetry.sequence, after_silence):
                continue  # Late or duplicate: never replaces newer data.
            if self.tracker.restarts != restarts:
                self._by_frame.clear()  # The old run reused these frame_ids.
            self.newest = telemetry
            self._by_frame[telemetry.frame_id] = telemetry
            self._by_frame.move_to_end(telemetry.frame_id)
            if len(self._by_frame) > self._kept:
                self._by_frame.popitem(last=False)

    def for_frame(self, frame_id: int):
        """Returns the newest packet sent at or before frame_id, or None."""
        earlier = [key for key in self._by_frame if key <= frame_id]
        return self._by_frame[max(earlier)] if earlier else None
