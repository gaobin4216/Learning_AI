@echo off
net session >nul 2>&1
if %errorLevel% neq 0 (
    powershell -Command "Start-Process '%~f0' -Verb RunAs -Wait"
    type "%~dp0test_output.txt" 2>nul
    exit /b
)
cscript //nologo "%~dp0test_template.vbs" > "%~dp0test_output.txt" 2>&1
