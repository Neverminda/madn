cmake_minimum_required(VERSION 4.0)
project(madn)

set(CMAKE_CXX_STANDARD 23)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/Game.hpp
    src/Game.cpp
	src/statistics.hpp
)

# Create executable
add_executable(madn ${SOURCES})
