#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BIN_PATH="$BUILD_DIR/bin/WeatherMap"

echo "[1/4] Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "[2/4] Running CMake configuration..."
cmake -DCMAKE_BUILD_TYPE=Release ..

echo "[3/4] Building project..."
cmake --build . --config Release

echo ""
echo "[4/4] Running unit tests..."
ctest --output-on-failure

echo ""
echo "Build and tests finished successfully."

if [ -f "$BIN_PATH" ]; then
    echo "Launching WeatherMap..."
    echo ""
    "$BIN_PATH"
else
    echo "ERROR: Executable not found at:"
    echo "  $BIN_PATH"
    echo "Check your CMakeLists.txt for the correct output directory."
fi

