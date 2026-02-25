# V2V Database Engine Installation Guide

The V2V Database Engine is designed to be portable and easy to install across Windows, Linux, and macOS.

## 🚀 Quick Setup

### 🪟 Windows (Recommended)
1. Download the latest `v2vdb-windows.exe` from the [Releases](https://github.com/phravins/Database-Engine/releases/latest) page.
2. Place the executable in any folder.
3. Open a terminal (CMD or PowerShell) in that folder.
4. Run:
   ```cmd
   v2vdb-windows.exe
   ```
   *(Note: The binary is standalone and does not require additional DLLs).*

### 🐧 Linux
1. Download the `v2vdb-linux` binary from the [Releases](https://github.com/phravins/Database-Engine/releases/latest) page.
2. Grant execution permissions:
   ```bash
   chmod +x v2vdb-linux
   ```
3. Run the database:
   ```bash
   ./v2vdb-linux
   ```

### 🍎 macOS
1. Download the `v2vdb-macos` binary from the [Releases](https://github.com/phravins/Database-Engine/releases/latest) page.
2. Grant execution permissions:
   ```bash
   chmod +x v2vdb-macos
   ```
3. Run the database:
   ```bash
   ./v2vdb-macos
   ```

## 🔄 Auto-Update Feature
Once installed, you can keep your database engine up to date without manual downloads:
1. Open the interactive shell.
2. Type `update`.
3. The system will automatically check for the latest version, download the new binary, and restart the application for you.

## 🛡️ Security
The default credentials for the interactive shell are:
- **Username**: `admin`
- **Password**: `admin`

## 🛠️ Building from Source
If you prefer to build the project yourself:
1. Ensure you have `CMake` and a `C++17` compiler installed.
2. Run:
   ```bash
   cmake -B build
   cmake --build build --config Release
   ```
