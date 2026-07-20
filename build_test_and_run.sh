#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PRESET="default"
BUILD_DIR="$SCRIPT_DIR/build"
BIN_PATH="$BUILD_DIR/bin/WeatherMap"
BUILD_JOBS="${BUILD_JOBS:-1}"
TEST_JOBS="${TEST_JOBS:-1}"
RUN_TESTS="${RUN_TESTS:-1}"
RUN_APP="${RUN_APP:-1}"

echo "[1/4] Configuring project with CMake preset: $PRESET"
cmake --preset "$PRESET"

echo "[2/4] Building project..."
cmake --build --preset "$PRESET" --parallel "$BUILD_JOBS"

echo ""
if [ "$RUN_TESTS" = "1" ]; then
    echo "[3/4] Running unit tests..."
    ctest --preset "$PRESET" --parallel "$TEST_JOBS"
else
    echo "[3/4] Skipping unit tests (RUN_TESTS=$RUN_TESTS)"
fi

echo ""
echo "[4/4] Launching application..."
echo ""

echo "Build and tests finished successfully."

if [ "$RUN_APP" != "1" ]; then
    echo "Skipping app launch (RUN_APP=$RUN_APP)"
elif [ -f "$BIN_PATH" ]; then
    "$BIN_PATH"
else
    echo "ERROR: Executable not found at:"
    echo "  $BIN_PATH"
    echo "Check CMake preset binaryDir and target output settings."
fi

