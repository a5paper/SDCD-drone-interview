"""Receives the simulator's JPEG video: rebuilds frames from VideoChunks."""

import collections
import time

import cv2
import numpy as np
from google.protobuf import message

from protocols import video_pb2
from services import udp
from units import MS_PER_S


Frame = collections.namedtuple("Frame", ["frame_id", "timestamp_s", "image"])


class NoFecDecoder:
    """Ignores parity chunks: a frame needs all of its data chunks."""

    def recover(self, data: dict, parity: dict, chunk_count: int,
                group_size: int) -> None:
        """Leaves the data chunks unchanged."""


class XorFecDecoder:
    """Rebuilds one missing data chunk per group from the group's parity.

    The parity is the XOR of the group's chunks, each zero-padded to the
    longest, so XOR-ing it with the chunks that arrived gives the missing one.
    A rebuilt last chunk may carry padding; the frame is trimmed to frame_size.
    """

    def recover(self, data: dict, parity: dict, chunk_count: int,
                group_size: int) -> None:
        """Adds every data chunk that its group's parity can rebuild."""
        for group, payload in parity.items():
            end = min((group + 1) * group_size, chunk_count)
            members = range(group * group_size, end)
            missing = [i for i in members if i not in data]
            if len(missing) != 1:
                continue
            rebuilt = np.frombuffer(payload, np.uint8).copy()
            for i in members:
                if i != missing[0]:
                    rebuilt[:len(data[i])] ^= np.frombuffer(data[i], np.uint8)
            data[missing[0]] = rebuilt.tobytes()


DECODERS = {
    video_pb2.FEC_SCHEME_NONE: NoFecDecoder(),
    video_pb2.FEC_SCHEME_XOR: XorFecDecoder(),
}


class FrameAssembler:
    """Rebuilds frames from VideoChunk datagrams.

    A newer frame_id drops the frame in progress; chunks of older frames, and
    of a frame already returned, are ignored. A lower frame_id after the
    stream was silent, or one more than restart_gap_frames below the newest,
    means the sender restarted (or the newest was bogus), so the assembler
    resyncs to it. The decoder for each frame comes from its chunks'
    fec_scheme; use_fec=False ignores parity.
    """

    def __init__(self, use_fec: bool, restart_gap_frames: int) -> None:
        self.use_fec = use_fec
        self.restart_gap_frames = restart_gap_frames
        self.restarts = 0
        self.frame_id = -1
        self.data = {}
        self.parity = {}
        self.done = False
        self.rebuilt = False
        self.recovered = 0

    def add(self, datagram: bytes, after_silence: bool = False):
        """Returns a Frame when a frame completes, else None.

        after_silence: no datagram arrived for a while before this one (a
        late chunk does not arrive after that long, a restarted sender does).
        """
        chunk = video_pb2.VideoChunk()
        chunk.ParseFromString(datagram)
        if chunk.frame_id < self.frame_id:
            big_drop = self.frame_id - chunk.frame_id > self.restart_gap_frames
            if not (after_silence or big_drop):
                return None
            self.restarts += 1
        if chunk.frame_id != self.frame_id:
            self.frame_id = chunk.frame_id
            self.data, self.parity = {}, {}
            self.done = self.rebuilt = False
        if self.done:
            return None

        count = chunk.chunk_count
        if chunk.chunk_index < count:
            self.data[chunk.chunk_index] = chunk.payload
        else:
            self.parity[chunk.chunk_index - count] = chunk.payload
        if len(self.data) < count and self.use_fec:
            before = len(self.data)
            decoder = DECODERS.get(chunk.fec_scheme, NoFecDecoder())
            decoder.recover(self.data, self.parity, count, chunk.fec_group_size)
            self.rebuilt |= len(self.data) > before
        if len(self.data) < count:
            return None

        self.done = True
        self.recovered += self.rebuilt
        jpeg = b"".join(self.data[i] for i in range(count))
        encoded = np.frombuffer(jpeg[:chunk.frame_size], np.uint8)
        image = cv2.imdecode(encoded, cv2.IMREAD_COLOR)
        if image is None:
            return None
        return Frame(chunk.frame_id, chunk.timestamp_s, image)


class VideoReceiver:
    """Listens on the video port and returns the frames completed so far.

    Anyone can send to a UDP port, so a datagram that is not a valid chunk
    (bad protobuf, inconsistent sizes, undecodable JPEG) is counted in
    rejected and skipped; it never stops the receiver.
    """

    def __init__(self, address: str, port: int, use_fec: bool,
                 restart_gap_frames: int, silence_ms: int) -> None:
        self._sock = udp.open_receiver(address, port)
        self.assembler = FrameAssembler(use_fec, restart_gap_frames)
        self._silence_s = silence_ms / MS_PER_S
        self._last_arrival_s = None
        self.rejected = 0

    def poll(self) -> list:
        """Returns the frames completed since the last call, oldest first."""
        frames = []
        datagrams = udp.drain(self._sock)
        if datagrams:
            now_s = time.monotonic()
            last_s = self._last_arrival_s
            after_silence = (last_s is not None and
                             now_s - last_s > self._silence_s)
            self._last_arrival_s = now_s
        for datagram in datagrams:
            try:
                frame = self.assembler.add(datagram, after_silence)
                after_silence = False  # Only the first datagram of the batch.
            except (message.DecodeError, cv2.error, ValueError):
                self.rejected += 1
                continue
            if frame is not None:
                frames.append(frame)
        return frames
