@echo off
echo ========================================
echo Building v2vdb for Windows
echo ========================================

REM Clean
if exist build rmdir /s /q build
mkdir build

REM Build with your existing tools
cd build

REM If using CMake:
cmake -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=make -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release


REM Copy the executable
mkdir ..\releases\windows 2>nul
copy mydb.exe ..\releases\windows\v2vdb.exe

cd ..
echo.
echo Windows build complete: releases/windows/v2vdb.exe
