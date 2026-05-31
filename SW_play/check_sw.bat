@echo off
net session >nul 2>&1
if %errorLevel% neq 0 (
    powershell -Command "Start-Process '%~f0' -Verb RunAs -Wait"
    type "%~dp0check_result.txt" 2>nul
    exit /b
)
cscript //nologo "%~dp0check_sw.vbs" > "%~dp0check_result.txt" 2>&1
