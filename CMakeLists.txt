cmake_minimum_required(VERSION 3.15)
project(TankGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
include_directories(include/common)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(TankGame ${SOURCES})
