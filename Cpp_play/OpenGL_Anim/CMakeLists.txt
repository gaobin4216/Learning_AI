cmake_minimum_required(VERSION 3.20)
project(OpenGL_Anim VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- Dependencies ----------

include(FetchContent)

# GLFW - window management & OpenGL context
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glfw)

# ---------- Executable ----------

add_executable(anim
    src/main.cpp
    src/glad.c
    src/shader.cpp
    src/mesh.cpp
)

target_include_directories(anim PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(anim PRIVATE
    glfw
    opengl32
)

# Copy shaders to build dir
file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/shaders/*")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    configure_file(${SHADER} ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME} COPYONLY)
endforeach()
