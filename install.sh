#!/bin/bash

echo "=========================================="
echo "     V2V Database Installer (Linux/macOS)"
echo "=========================================="

# Create build directory
mkdir -p build
cd build

# Configure for Release and Install to ../install
echo "[1/2] Configuring..."
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install ..
if [ $? -ne 0 ]; then
    echo "Configuration failed."
    exit 1
fi

# Build and Install
echo "[2/2] Installing..."
cmake --build . --config Release --target install
if [ $? -ne 0 ]; then
    echo "Installation failed."
    exit 1
fi

cd ..
echo ""
echo "=========================================="
echo "     SUCCESS! V2V Database Installed."
echo "=========================================="
echo "Run the database with:"
echo "   ./install/bin/mydb"
echo ""
