@echo off
rem Starts SCAPMS. MSYS2 goes first on PATH so the program finds the right Qt / MinGW DLLs.
rem Any arguments are passed on, e.g.:  run.bat --screenshots docs\screenshots
rem Author: Linda Obodo (COS202 Lab Assessment)
setlocal
set "PATH=C:\msys64\ucrt64\bin;%PATH%"
cd /d "%~dp0"
if not exist build\SCAPMS.exe (
    echo SCAPMS.exe not found - run build.bat first.
    exit /b 1
)
start "" /wait build\SCAPMS.exe %*
