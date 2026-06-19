cmake_minimum_required(VERSION 3.10)
project(WeatherCLI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find curl library
find_package(CURL REQUIRED)

# Create executable
add_executable(weather_cli main.cpp)

# Link libraries
target_link_libraries(weather_cli PRIVATE CURL::libcurl)

# nlohmann-json is header-only, just need to include it
target_link_libraries(weather_cli PRIVATE nlohmann_json::nlohmann_json)
