"""Imports the protobuf messages that the build generates.

compile.sh writes them to build/protocols, compile.ps1 to
build-windows/protocols.
"""

import pathlib
import sys

_REPO_DIR = pathlib.Path(__file__).resolve().parents[1]
if sys.platform == "win32":
    PROTOCOLS_DIR = _REPO_DIR / "build-windows" / "protocols"
    _COMPILE_COMMAND = "compile.ps1"
else:
    PROTOCOLS_DIR = _REPO_DIR / "build" / "protocols"
    _COMPILE_COMMAND = "./compile.sh"
sys.path.insert(0, str(PROTOCOLS_DIR))
try:
    import commands_pb2  # noqa: E402
    import telemetry_pb2  # noqa: E402
    import video_pb2  # noqa: E402
except ImportError as error:
    sys.exit(f"Cannot import the protobuf messages from {PROTOCOLS_DIR} "
             f"({error}). Run {_COMPILE_COMMAND} first.")

__all__ = ["commands_pb2", "telemetry_pb2", "video_pb2"]
