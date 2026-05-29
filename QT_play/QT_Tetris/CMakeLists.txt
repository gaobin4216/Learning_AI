cmake_minimum_required(VERSION 3.16)
project(QT_Tetris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(QT_Tetris WIN32
    main.cpp
    TetrisGame.h
    TetrisGame.cpp
)

target_link_libraries(QT_Tetris PRIVATE Qt6::Widgets)
