"""Imports the protobuf messages that compile.sh builds in build/protocols."""

import pathlib
import sys

PROTOCOLS_DIR = (pathlib.Path(__file__).resolve().parents[1] / "build" /
                 "protocols")
sys.path.insert(0, str(PROTOCOLS_DIR))
try:
    import commands_pb2  # noqa: E402
    import telemetry_pb2  # noqa: E402
    import video_pb2  # noqa: E402
except ImportError as error:
    sys.exit(f"Cannot import the protobuf messages from {PROTOCOLS_DIR} "
             f"({error}). Run ./compile.sh first.")

__all__ = ["commands_pb2", "telemetry_pb2", "video_pb2"]
