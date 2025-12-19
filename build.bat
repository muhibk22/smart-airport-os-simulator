@echo off
echo ╔═══════════════════════════════════════════════════════════════╗
echo ║       Smart Airport OS Simulator - Build Script              ║
echo ╚═══════════════════════════════════════════════════════════════╝
echo.

REM Create build directory
if not exist build mkdir build
cd build

echo [1/3] Configuring with CMake...
cmake .. -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo [2/3] Building project...
cmake --build .
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo [3/3] Build complete!
echo.
echo Available executables:
echo   - SmartAirportOS.exe  (main simulator)
echo   - simple_test.exe     (basic pthread test)
echo.
echo To run simple test:
echo   cd build
echo   simple_test.exe
echo.
pause
