@echo off
echo ==========================================
echo      V2V Database Deployment Script
echo ==========================================

rmdir /s /q build_release
mkdir build_release
cd build_release

echo [1/3] Configuring Release Build...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
if %errorlevel% neq 0 exit /b %errorlevel%

echo [2/3] Building...
cmake --build . --config Release
if %errorlevel% neq 0 exit /b %errorlevel%

echo [3/3] Packaging...
cpack -C Release
if %errorlevel% neq 0 exit /b %errorlevel%

echo ==========================================
echo      SUCCESS! Package created.
echo ==========================================
dir *.zip
cd ..
