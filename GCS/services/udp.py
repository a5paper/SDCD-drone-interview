"""Non-blocking UDP helpers, so the Tk event loop never waits on a socket."""

import socket

MAX_DATAGRAM = 65535  # UDP maximum; nothing the simulator sends is larger.


def open_receiver(address: str, port: int) -> socket.socket:
    """Returns a non-blocking UDP socket bound to the address and port."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((address, port))
    sock.setblocking(False)
    return sock


def drain(sock: socket.socket) -> list:
    """Returns every datagram waiting on the socket, oldest first."""
    datagrams = []
    while True:
        try:
            datagrams.append(sock.recv(MAX_DATAGRAM))
        except BlockingIOError:
            return datagrams
