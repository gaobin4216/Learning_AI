cmake_minimum_required(VERSION 3.20)
project(MiniKV VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Third-party libraries (6 total)
# ============================================================

# --- System libraries (find_package) ---

# 4. SQLite3 - embedded database
find_package(SQLite3 REQUIRED)

# 5. zlib - data compression
find_package(ZLIB REQUIRED)

# --- FetchContent libraries ---

include(FetchContent)

# 1. spdlog - structured logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
)

# 2. nlohmann/json - JSON serialization & config
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)

# 3. cxxopts - CLI argument parsing
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG        v3.2.1
    GIT_SHALLOW    TRUE
)

# 6. fmt - text formatting
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        11.1.4
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(spdlog nlohmann_json cxxopts fmt)

# ============================================================
# Main executable
# ============================================================

add_executable(minikv
    src/main.cpp
    src/app.cpp
    src/database.cpp
    src/compressor.cpp
)

target_include_directories(minikv PRIVATE src)

target_link_libraries(minikv PRIVATE
    spdlog::spdlog              # 1. logging
    nlohmann_json::nlohmann_json # 2. JSON
    cxxopts::cxxopts             # 3. CLI parsing
    SQLite::SQLite3              # 4. database
    ZLIB::ZLIB                   # 5. compression
    fmt::fmt                     # 6. formatting
)

# Copy default config to build dir
configure_file(
    ${CMAKE_SOURCE_DIR}/config/default.json
    ${CMAKE_BINARY_DIR}/config/default.json
    COPYONLY
)
