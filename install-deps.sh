#!/usr/bin/env bash
# Installs the system packages needed to build the project (Ubuntu/Debian).
# Re-run under bash if invoked via `sh script.sh` (dash lacks pipefail etc.)
if [ -z "${BASH_VERSION:-}" ]; then exec bash "$0" "$@"; fi
set -euo pipefail

SUDO=""
if [[ ${EUID} -ne 0 ]]; then
  SUDO="sudo"
fi

${SUDO} apt-get update
${SUDO} apt-get install -y \
  build-essential \
  libgtest-dev \
  gdb \
  valgrind \
  cmake \
  protobuf-compiler \
  libprotobuf-dev \
  libopencv-dev \
  python3-venv \
  python3-tk

# The GCS's Python packages go into its own venv. tkinter is not on PyPI, so
# the venv uses the system python3, which python3-tk provides it for.
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENV_DIR="${ROOT_DIR}/GCS/.venv"
python3 -m venv "${VENV_DIR}"
"${VENV_DIR}/bin/pip" install --quiet -r "${ROOT_DIR}/GCS/requirements-dev.txt"

echo "Dependencies installed."
echo "  GCS venv: ${VENV_DIR} ($("${VENV_DIR}/bin/python" --version))"
echo "  cmake:  $(cmake --version | head -n1)"
echo "  protoc: $(protoc --version)"
echo "  opencv: $(pkg-config --modversion opencv4)"
echo "  gdb:    $(gdb --version | head -n1)"
echo "  valgrind: $(valgrind --version)"

