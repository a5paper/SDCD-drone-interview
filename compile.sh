#!/usr/bin/env bash
# Configures and builds the project. CMake generates the protobuf sources.
# Usage: ./compile.sh [Debug|Release] [--clean]
# Re-run under bash if invoked via `sh script.sh` (dash lacks pipefail etc.)
if [ -z "${BASH_VERSION:-}" ]; then exec bash "$0" "$@"; fi
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

BUILD_TYPE="Debug"
CLEAN=0
for arg in "$@"; do
  case "${arg}" in
    Debug|Release|RelWithDebInfo|MinSizeRel) BUILD_TYPE="${arg}" ;;
    --clean) CLEAN=1 ;;
    -h|--help)
      echo "Usage: $0 [Debug|Release|RelWithDebInfo|MinSizeRel] [--clean]"
      exit 0 ;;
    *) echo "Unknown argument: ${arg}" >&2; exit 1 ;;
  esac
done

for tool in cmake protoc; do
  if ! command -v "${tool}" >/dev/null 2>&1; then
    echo "Error: '${tool}' not found. Run ./install-deps.sh first." >&2
    exit 1
  fi
done

if [[ ${CLEAN} -eq 1 ]]; then
  echo "Cleaning build directory..."
  rm -rf "${BUILD_DIR}"
fi

echo "Configuring (${BUILD_TYPE})..."
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo "Building..."
cmake --build "${BUILD_DIR}" --parallel "$(nproc)"

echo "Build complete: ${BUILD_DIR}/DroneSimulator/DroneSimulator"
