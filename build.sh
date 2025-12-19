#!/bin/bash

echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║       Smart Airport OS Simulator - Build Script (Linux)      ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""

# Create build directory
mkdir -p build
cd build

echo "[1/3] Configuring with CMake..."
cmake ..
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

echo ""
echo "[2/3] Building project..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

echo ""
echo "[3/3] Build complete!"
echo ""
echo "Available executables:"
echo "  - SmartAirportOS  (main simulator)"
echo "  - simple_test     (basic pthread test)"
echo ""
echo "To run simple test:"
echo "  cd build"
echo "  ./simple_test"
echo ""
