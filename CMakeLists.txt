cmake_minimum_required(VERSION 3.16)
project(EndlessRunner3D VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(SRC_FILES
    src/main.cpp
    src/shader.cpp
    src/mesh.cpp
    src/object.cpp
    src/collision.cpp
    src/player.cpp
    src/game.cpp
)

add_executable(endless-runner ${SRC_FILES})

target_include_directories(endless-runner PRIVATE include)

target_link_libraries(endless-runner
    OpenGL::GL
    GLEW::GLEW
    glfw
    glm::glm
)
