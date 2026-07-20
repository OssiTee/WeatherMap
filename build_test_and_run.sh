#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PRESET="default"
BUILD_DIR="$SCRIPT_DIR/build"
BIN_PATH="$BUILD_DIR/bin/WeatherMap"

echo "[1/4] Configuring project with CMake preset: $PRESET"
cmake --preset "$PRESET"

echo "[2/4] Building project..."
cmake --build --preset "$PRESET"

echo ""
echo "[3/4] Running unit tests..."
ctest --preset "$PRESET"

echo ""
echo "[4/4] Launching application..."
echo ""

echo "Build and tests finished successfully."

if [ -f "$BIN_PATH" ]; then
    "$BIN_PATH"
else
    echo "ERROR: Executable not found at:"
    echo "  $BIN_PATH"
    echo "Check CMake preset binaryDir and target output settings."
fi

