#!/usr/bin/env bash
# Runs the whole system: the simulator in the background, the GCS in front.
# Closing the GCS (or Ctrl+C) stops the simulator. Builds first if needed.
# Usage: ./run.sh [Debug|Release] [--config PATH]   (default Release)
# Re-run under bash if invoked via `sh script.sh` (dash lacks pipefail etc.)
if [ -z "${BASH_VERSION:-}" ]; then exec bash "$0" "$@"; fi
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
SIMULATOR="${BUILD_DIR}/DroneSimulator/DroneSimulator"
GCS_PYTHON="${ROOT_DIR}/GCS/.venv/bin/python"
SIMULATOR_LOG="${BUILD_DIR}/simulator.log"
STARTUP_CHECK_S=1

BUILD_TYPE="Release"
CONFIG_ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    Debug|Release) BUILD_TYPE="$1"; shift ;;
    --config)
      [[ $# -ge 2 ]] || { echo "--config needs a path" >&2; exit 1; }
      CONFIG_ARGS=(--config "$2"); shift 2 ;;
    -h|--help)
      echo "Usage: $0 [Debug|Release] [--config PATH]"
      exit 0 ;;
    *) echo "Unknown argument: $1" >&2; exit 1 ;;
  esac
done

if [[ ! -x "${GCS_PYTHON}" ]]; then
  echo "Error: GCS venv not found. Run ./install-deps.sh first." >&2
  exit 1
fi

# Build when the simulator is missing or was built with another build type.
built_type=""
if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  built_type="$(sed -n 's/^CMAKE_BUILD_TYPE:STRING=//p' \
    "${BUILD_DIR}/CMakeCache.txt")"
fi
if [[ ! -x "${SIMULATOR}" || "${built_type}" != "${BUILD_TYPE}" ]]; then
  bash "${ROOT_DIR}/compile.sh" "${BUILD_TYPE}"
fi

"${SIMULATOR}" "${CONFIG_ARGS[@]}" >"${SIMULATOR_LOG}" 2>&1 &
SIMULATOR_PID=$!
trap 'kill "${SIMULATOR_PID}" 2>/dev/null || true' EXIT
echo "Simulator running (PID ${SIMULATOR_PID}, log: ${SIMULATOR_LOG})"

# A simulator that cannot open its sockets exits at once: report it.
sleep "${STARTUP_CHECK_S}"
if ! kill -0 "${SIMULATOR_PID}" 2>/dev/null; then
  echo "Error: the simulator stopped. Last lines of ${SIMULATOR_LOG}:" >&2
  tail -n 5 "${SIMULATOR_LOG}" >&2
  exit 1
fi

"${GCS_PYTHON}" "${ROOT_DIR}/GCS/main.py" "${CONFIG_ARGS[@]}"
