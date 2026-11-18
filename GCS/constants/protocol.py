"""Wire-format facts and units shared with the simulator; not settings."""

MAX_DATAGRAM = 65535  # UDP maximum; nothing the simulator sends is larger.
MS_PER_S = 1000.0
MAX_PERCENT = 100  # A command axis of 1.0 is 100 % of the drone's speed.
STOP_COMMAND = (0.0, 0.0)  # (forward, right) velocity of a stopped drone.
BACKOFF_FACTOR = 2  # The resend interval doubles while the link is LOST.
GPS_STATE_ONEOF = "state"
GPS_FIX = "fix"
GPS_NO_FIX = "no_fix"
