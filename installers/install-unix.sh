#!/bin/bash

VERSION="1.0.7"
GITHUB_USER="phravins"
REPO="Database-Engine"
INSTALL_DIR="/usr/local/bin"

# Detect OS
OS=$(uname -s | tr '[:upper:]' '[:lower:]')

echo "========================================"
echo "   v2vdb Database Installer"
echo "========================================"

# Check root
if [[ $EUID -ne 0 ]]; then
   echo "ERROR: Run with sudo"
   exit 1
fi

echo "[1/3] Downloading v2vdb for $OS..."
URL="https://github.com/$GITHUB_USER/$REPO/releases/download/v${VERSION}/v2vdb-${OS}"
TEMP_FILE="/tmp/v2vdb"

if command -v curl &> /dev/null; then
    curl -L -f "$URL" -o "$TEMP_FILE"
else
    wget "$URL" -O "$TEMP_FILE"
fi

# Check file size (should be > 1KB)
FILE_SIZE=$(wc -c < "$TEMP_FILE" | tr -d ' ')
if [[ $FILE_SIZE -lt 1024 ]]; then
    echo "ERROR: Downloaded file is too small ($FILE_SIZE bytes). Likely a 404 error."
    echo "Please ensure release v$VERSION exists on GitHub with 'v2vdb-$OS' asset."
    rm "$TEMP_FILE"
    exit 1
fi

if [[ ! -f "$TEMP_FILE" ]]; then
    echo "ERROR: Download failed!"
    exit 1
fi

echo "[2/3] Installing v2vdb..."
chmod +x "$TEMP_FILE"
mv "$TEMP_FILE" "$INSTALL_DIR/v2vdb"

echo "[3/3] Verifying installation..."
if command -v v2vdb &> /dev/null; then
    echo ""
    echo "========================================"
    echo "   Installation Complete!"
    echo "========================================"
    echo ""
    echo "Type: v2vdb"
else
    echo "ERROR: Installation failed"
    exit 1
fi
