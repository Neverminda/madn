cmake_minimum_required(VERSION 4.0)
project(madn)

set(CMAKE_CXX_STANDARD 23)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/GameRunner.cpp
    src/Player.cpp
)

# Create executable
add_executable(madn ${SOURCES})