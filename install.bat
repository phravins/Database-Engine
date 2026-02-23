@echo off
echo ==========================================
echo      V2V Database Installer
echo ==========================================

:: Create build directory
if not exist build mkdir build
cd build

:: Configure for Release and Install to ../install
echo [1/2] Configuring...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install ..
if %errorlevel% neq 0 (
    echo Configuration failed.
    exit /b %errorlevel%
)

:: Build and Install
echo [2/2] Installing...
cmake --build . --config Release --target install
if %errorlevel% neq 0 (
    echo Installation failed.
    exit /b %errorlevel%
)

cd ..
echo.
echo ==========================================
echo      SUCCESS! V2V Database Installed.
echo ==========================================
echo Run the database with:
echo    install\bin\mydb.exe
echo.
