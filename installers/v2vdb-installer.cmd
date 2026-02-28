: '
@echo off
goto :WINDOWS
'

# ==============================================================================
#  LINUX / MACOS (Bash)
# ==============================================================================
VERSION="1.0.7"
GITHUB_USER="phravins"
REPO="Database-Engine"
INSTALL_DIR="/usr/local/bin"

# Detect OS
OS=$(uname -s | tr '[:upper:]' '[:lower:]')

echo "========================================"
echo "   v2vdb Database Installer (Unix)"
echo "========================================"

# Check root
if [[ $EUID -ne 0 ]]; then
   echo "ERROR: Run with sudo"
   exit 1
fi

echo "[1/3] Downloading v2vdb for $OS..."
if [[ "$OS" == "darwin" ]]; then
    OS="macos"
fi
URL="https://github.com/$GITHUB_USER/$REPO/releases/latest/download/v2vdb-${OS}"
TEMP_FILE="/tmp/v2vdb"

if command -v curl &> /dev/null; then
    curl -L "$URL" -o "$TEMP_FILE"
else
    wget "$URL" -O "$TEMP_FILE"
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

# Exit Bash to prevent running Windows code
exit 0


:WINDOWS
REM ============================================================================
REM  WINDOWS (Batch)
REM ============================================================================
echo ========================================
echo   v2vdb Database - Windows Installer
echo ========================================

REM Config
set VERSION=1.0.7
set GITHUB_USER=phravins
set REPO=Database-Engine
set INSTALL_DIR=%ProgramFiles%\v2vdb

REM Check admin
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Run as Administrator!
    pause
    exit /b 1
)

echo [1/4] Downloading v2vdb...
set URL=https://github.com/%GITHUB_USER%/%REPO%/releases/latest/download/v2vdb-windows.exe
set TEMP_FILE=%TEMP%\v2vdb.exe

where curl >nul 2>&1
if %errorlevel% equ 0 (
    curl -L -f "%URL%" -o "%TEMP_FILE%"
) else (
    powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri '%URL%' -OutFile '%TEMP_FILE%'"
)

if not exist "%TEMP_FILE%" (
    echo ERROR: Download failed!
    pause
    exit /b 1
)

for %%I in ("%TEMP_FILE%") do set SIZE=%%~zI
if "%SIZE%"=="" set SIZE=0
if %SIZE% LSS 1024 (
    echo ERROR: Downloaded file is too small (%SIZE% bytes). Likely a 404 error.
    echo Please ensure release v%VERSION% exists on GitHub with 'v2vdb-windows.exe' asset.
    del "%TEMP_FILE%"
    pause
    exit /b 1
)

echo [2/4] Creating installation directory...
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

echo [3/4] Installing v2vdb...
copy /Y "%TEMP_FILE%" "%INSTALL_DIR%\v2vdb.exe"

echo [4/4] Adding to PATH...
set "PATH_TO_ADD=%INSTALL_DIR%"
for /f "tokens=2,*" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path') do set "KEY_VALUE=%%B"
echo %KEY_VALUE% | findstr /C:"%PATH_TO_ADD%" >nul 2>&1
if errorlevel 1 (
    setx /M PATH "%KEY_VALUE%;%PATH_TO_ADD%"
    echo Added %PATH_TO_ADD% to PATH.
) else (
    echo Path already exists.
)

echo.
echo ========================================
echo   Installation Complete!
echo ========================================
echo.
echo Open a NEW command prompt and type: v2vdb
echo.
pause
