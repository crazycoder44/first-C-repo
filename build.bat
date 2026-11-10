@echo off
rem Builds SCAPMS with the MSYS2 UCRT64 toolchain (MinGW g++, CMake, Ninja, Qt 6).
rem MSYS2 is put FIRST on PATH so that DLLs from other programs (for example PostgreSQL,
rem which ships its own older zlib1.dll / libzstd.dll) cannot break the compiler.
rem Author: Linda Obodo (COS202 Lab Assessment)
setlocal
set "PATH=C:\msys64\ucrt64\bin;%PATH%"
cd /d "%~dp0"
if not exist build\build.ninja (
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 || exit /b 1
)
cmake --build build || exit /b 1
echo.
echo Build finished: build\SCAPMS.exe   (start it with run.bat)
