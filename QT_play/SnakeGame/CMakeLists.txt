cmake_minimum_required(VERSION 3.16)
project(SnakeGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(SnakeGame
    main.cpp
    mainwindow.h mainwindow.cpp
    gamewidget.h gamewidget.cpp
)

target_link_libraries(SnakeGame PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
