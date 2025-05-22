cmake_minimum_required(VERSION 3.22)
project(TankGame C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(common)

add_executable(TankGame
        common/ActionRequest.h)
