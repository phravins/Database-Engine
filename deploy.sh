#!/bin/bash

echo "=========================================="
echo "     V2V Database Deployment Script (Unix)"
echo "=========================================="

rm -rf build_release
mkdir -p build_release
cd build_release

echo "[1/3] Configuring Release Build..."
cmake -DCMAKE_BUILD_TYPE=Release ..
if [ $? -ne 0 ]; then
    exit 1
fi

echo "[2/3] Building..."
cmake --build . --config Release
if [ $? -ne 0 ]; then
    exit 1
fi

echo "[3/3] Packaging..."
cpack -C Release
if [ $? -ne 0 ]; then
    exit 1
fi

echo "=========================================="
echo "     SUCCESS! Package created."
echo "=========================================="
ls *.zip *.tar.gz 2>/dev/null
cd ..
