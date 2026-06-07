@echo off
chcp 65001 >nul
echo 正在激活虚拟环境并启动贪吃蛇游戏...
call "%~dp0venv\Scripts\activate.bat"
python "%~dp0main.py"
pause
