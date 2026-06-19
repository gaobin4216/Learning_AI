# Weather CLI

一个简单的命令行天气查询工具，使用 C++ 编写。

## 功能

- 查询指定城市的当前天气
- 显示温度、湿度、风速等信息
- 显示3天天气预报

## 依赖

- libcurl - HTTP 请求库
- nlohmann-json - JSON 解析库

## 安装依赖 (MSYS2 MinGW64)

```bash
pacman -S mingw-w64-x86_64-curl mingw-w64-x86_64-nlohmann-json
```

## 编译

### 方法一：使用构建脚本

```bash
# Linux/MSYS2 环境
./build.sh

# Windows CMD
build.bat
```

### 方法二：手动编译

```bash
# 通过 MSYS2 shell 编译
/c/msys64/usr/bin/bash -lc "cd '$(pwd)' && g++ -std=c++17 -o weather_cli.exe main.cpp -lcurl"
```

## 使用

```bash
# 命令行参数方式
./weather_cli.exe Beijing

# 交互式输入
./weather_cli.exe
Enter city name: Shanghai
```

## 示例输出

```
Fetching weather for Beijing...

===== Weather for Beijing =====
Temperature: 22°C (72°F)
Condition:   Light Rain Shower
Humidity:    94%
Wind Speed:  6 km/h

----- 3-Day Forecast -----
2026-06-19: 23°C ~ 27°C, Smoky haze
2026-06-20: 22°C ~ 31°C, Sunny
2026-06-21: 20°C ~ 33°C, Sunny
===========================
```
