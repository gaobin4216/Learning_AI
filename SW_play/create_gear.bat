@echo off
echo ========================================
echo  SolidWorks 2022 Gear Generator
echo ========================================
echo.

:: Check admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Requesting admin privileges...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

echo Running with admin privileges.
echo.
echo Step 1: Checking SolidWorks...

:: Check if SolidWorks is running
tasklist /FI "IMAGENAME eq SLDWORKS.exe" | find /I "SLDWORKS" >nul 2>&1
if %errorLevel% neq 0 (
    echo SolidWorks not running, starting it...
    start "" "C:\Software\sw2022\SOLIDWORKS\sldworks.exe"
    echo Waiting 15 seconds for SolidWorks to load...
    timeout /t 15 /nobreak >nul
) else (
    echo SolidWorks is already running.
)

echo.
echo Step 2: Running gear script...
cscript //nologo "%~dp0create_gear.vbs"

echo.
echo Done! Check SolidWorks for the gear part.
pause
