#!/bin/bash

echo "========================================"
echo "Building v2vdb"
echo "========================================"

# Detect OS
OS=$(uname -s | tr '[:upper:]' '[:lower:]')

# Clean
rm -rf build
mkdir build
cd build

# Build using CMake
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Copy executable
mkdir -p ../releases/$OS
cp mydb ../releases/$OS/v2vdb

cd ..
echo ""
echo "$OS build complete: releases/$OS/v2vdb"
