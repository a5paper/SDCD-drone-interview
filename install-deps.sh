#!/usr/bin/env bash
# Installs the system packages needed to build the project (Ubuntu/Debian).
# Re-run under bash if invoked via `sh script.sh` (dash lacks pipefail etc.)
if [ -z "${BASH_VERSION:-}" ]; then exec bash "$0" "$@"; fi
set -euo pipefail

SUDO=""
if [[ $EUID -ne 0 ]]; then
    SUDO="sudo"
fi

$SUDO apt-get update
$SUDO apt-get install -y \
    build-essential \
    gdb \
    cmake \
    protobuf-compiler \
    libprotobuf-dev \
    libopencv-dev

echo "Dependencies installed."
echo "  cmake:  $(cmake --version | head -n1)"
echo "  protoc: $(protoc --version)"
echo "  opencv: $(pkg-config --modversion opencv4)"
echo "  gdb:    $(gdb --version | head -n1)"

