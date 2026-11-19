#!/usr/bin/env bash
# Runs the simulator's GoogleTest suite and the GCS's pytest suite.
# Usage: ./test.sh (run ./compile.sh first)
# Re-run under bash if invoked via `sh script.sh` (dash lacks pipefail etc.)
if [ -z "${BASH_VERSION:-}" ]; then exec bash "$0" "$@"; fi
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "C++ tests..."
ctest --test-dir "${ROOT_DIR}/build" --output-on-failure

echo "GCS tests..."
"${ROOT_DIR}/GCS/.venv/bin/pytest" "${ROOT_DIR}/GCS/tests"
