@echo off
echo ========================================
echo Building v2vdb with Visual Studio 2022
echo ========================================

REM Clean previous build attempts
if exist build rmdir /s /q build
mkdir build
cd build

REM Configure using Visual Studio 17 2022 generator
echo Configuring CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed.
    echo Ensure Visual Studio 2022 with C++ options is installed.
    exit /b 1
)

REM Build Release configuration
echo Building...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed.
    exit /b 1
)

REM Install/Copy executable
echo Copying binary...
mkdir ..\releases\windows 2>nul
copy Release\mydb.exe ..\releases\windows\v2vdb.exe

echo.
echo Build Complete: releases/windows/v2vdb.exe
cd ..
pause
