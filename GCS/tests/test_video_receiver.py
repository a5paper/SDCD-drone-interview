"""Tests frame reassembly and FEC recovery in services/video_receiver.py."""

import cv2
import numpy as np
import pytest

from protocols import video_pb2
from services.video_receiver import FrameAssembler, NoFecDecoder

CHUNK_BYTES = 1400  # The simulator's kMaxChunkPayloadBytes.
RESTART_GAP_FRAMES = 30
IMAGE_SIZE = 64
SEED = 0


@pytest.fixture(scope="module")
def jpeg():
    """A noisy JPEG big enough to need several chunks."""
    rng = np.random.default_rng(SEED)
    shape = (IMAGE_SIZE, IMAGE_SIZE, 3)
    image = rng.integers(0, 256, shape, dtype=np.uint8)
    encoded = cv2.imencode(".jpg", image)[1].tobytes()
    assert len(encoded) > 3 * CHUNK_BYTES
    return encoded


def xor_parity(chunks):
    """XOR of the chunks, each zero-padded to the longest."""
    parity = np.zeros(max(len(c) for c in chunks), np.uint8)
    for chunk in chunks:
        parity[:len(chunk)] ^= np.frombuffer(chunk, np.uint8)
    return parity.tobytes()


def make_datagrams(jpeg, frame_id, group_size=0, lost=()):
    """Serialized VideoChunks of one frame, as the simulator sends them.

    group_size 0 sends no parity; lost lists data chunk indices to drop.
    """
    data = [jpeg[i:i + CHUNK_BYTES] for i in range(0, len(jpeg), CHUNK_BYTES)]
    groups = [data[i:i + group_size]
              for i in range(0, len(data), group_size or len(data))]
    payloads = list(enumerate(data))
    if group_size:
        parity = [xor_parity(group) for group in groups]
        payloads += list(enumerate(parity, start=len(data)))
    scheme = (video_pb2.FEC_SCHEME_XOR if group_size
              else video_pb2.FEC_SCHEME_NONE)
    datagrams = []
    for index, payload in payloads:
        if index in lost:
            continue
        chunk = video_pb2.VideoChunk(
            frame_id=frame_id, chunk_index=index, chunk_count=len(data),
            frame_size=len(jpeg), payload=payload, timestamp_s=frame_id / 30,
            fec_scheme=scheme, fec_group_size=group_size)
        datagrams.append(chunk.SerializeToString())
    return datagrams


def feed(assembler, datagrams, after_silence=False):
    """Adds the datagrams in order; returns the frames that completed."""
    frames = [assembler.add(d, after_silence) for d in datagrams]
    return [f for f in frames if f is not None]


def chunk_count(jpeg):
    """Number of data chunks the frame splits into."""
    return -(-len(jpeg) // CHUNK_BYTES)


def test_complete_frame_is_decoded_once(jpeg):
    # Parity arriving after all data chunks must not return the frame again.
    datagrams = make_datagrams(jpeg, 7, group_size=2)
    assembler = FrameAssembler(True, RESTART_GAP_FRAMES)
    frames = feed(assembler, datagrams)
    assert len(frames) == 1
    assert frames[0].frame_id == 7
    assert frames[0].image.shape == (IMAGE_SIZE, IMAGE_SIZE, 3)
    assert assembler.recovered == 0


@pytest.mark.parametrize("lost", ["first", "last"])
def test_xor_rebuilds_one_lost_chunk(jpeg, lost):
    # The last chunk is shorter, so its rebuild carries zero padding.
    index = 0 if lost == "first" else chunk_count(jpeg) - 1
    group_size = chunk_count(jpeg)
    datagrams = make_datagrams(jpeg, 1, group_size, lost={index})
    assembler = FrameAssembler(True, RESTART_GAP_FRAMES)
    frames = feed(assembler, datagrams)
    assert len(frames) == 1
    assert assembler.recovered == 1
    encoded = np.frombuffer(jpeg, np.uint8)
    original = cv2.imdecode(encoded, cv2.IMREAD_COLOR)
    assert np.array_equal(frames[0].image, original)


def test_two_losses_in_one_group_drop_the_frame(jpeg):
    datagrams = make_datagrams(jpeg, 1, chunk_count(jpeg), lost={0, 1})
    assembler = FrameAssembler(True, RESTART_GAP_FRAMES)
    assert feed(assembler, datagrams) == []


def test_one_loss_per_group_is_rebuilt(jpeg):
    datagrams = make_datagrams(jpeg, 1, group_size=2, lost={0, 2})
    assembler = FrameAssembler(True, RESTART_GAP_FRAMES)
    assert len(feed(assembler, datagrams)) == 1


def test_fec_off_ignores_parity(jpeg):
    datagrams = make_datagrams(jpeg, 1, chunk_count(jpeg), lost={0})
    assembler = FrameAssembler(False, RESTART_GAP_FRAMES)
    assert feed(assembler, datagrams) == []


def test_no_fec_decoder_leaves_data_unchanged():
    data = {0: b"ab"}
    NoFecDecoder().recover(data, {0: b"xy"}, chunk_count=2, group_size=2)
    assert data == {0: b"ab"}


def test_newer_frame_drops_incomplete_one(jpeg):
    assembler = FrameAssembler(True, RESTART_GAP_FRAMES)
    old = make_datagrams(jpeg, frame_id=1)
    assert feed(assembler, old[:1]) == []
    frames = feed(assembler, make_datagrams(jpeg, frame_id=2))
    assert [f.frame_id for f in frames] == [2]
    assert feed(assembler, old[1:]) == []


def test_late_chunk_of_older_frame_is_ignored(jpeg):
    assembler = FrameAssembler(True, RESTART_GAP_FRAMES)
    feed(assembler, make_datagrams(jpeg, frame_id=10))
    assert feed(assembler, make_datagrams(jpeg, frame_id=9)) == []
    assert assembler.restarts == 0


NEWEST_ID = RESTART_GAP_FRAMES + 20


@pytest.mark.parametrize("old_id, after_silence", [
    (NEWEST_ID - 5, True),  # Small drop, but the stream was silent.
    (NEWEST_ID - RESTART_GAP_FRAMES - 1, False),  # Drop beyond the gap.
])
def test_restart_resyncs_to_lower_frame_id(jpeg, old_id, after_silence):
    newest = NEWEST_ID
    assembler = FrameAssembler(True, RESTART_GAP_FRAMES)
    feed(assembler, make_datagrams(jpeg, frame_id=newest))
    frames = feed(assembler, make_datagrams(jpeg, old_id), after_silence)
    assert [f.frame_id for f in frames] == [old_id]
    assert assembler.restarts == 1


FORGED_INDEX_PAST_COUNT = 1
FORGED_EXTRA_CHUNKS = 5


def test_real_frame_after_forged_count_is_rebuilt(jpeg):
    # The forged chunk must neither block the real frame nor fill its slots.
    count = chunk_count(jpeg)
    forged = video_pb2.VideoChunk(
        frame_id=1, chunk_index=count + FORGED_INDEX_PAST_COUNT,
        chunk_count=count + FORGED_EXTRA_CHUNKS, payload=b"x")
    datagrams = [forged.SerializeToString()]
    datagrams += make_datagrams(jpeg, 1, count, lost={0})
    assembler = FrameAssembler(True, RESTART_GAP_FRAMES)
    frames = feed(assembler, datagrams)
    assert len(frames) == 1
    assert assembler.recovered == 1
