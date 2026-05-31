cmake_minimum_required(VERSION 3.16)
project(QT_Firework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(QT_Firework WIN32
    main.cpp
    FireworkWidget.h
    FireworkWidget.cpp
)

target_link_libraries(QT_Firework PRIVATE Qt6::Widgets)
