# Minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(KillProcesses VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable target
add_executable(rkill main.cpp)

# Add compile options for more warnings (optional)
target_compile_options(rkill PRIVATE -Wall -Wextra -pedantic)
