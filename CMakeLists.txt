cmake_minimum_required(VERSION 3.22)
project(Z_RPC C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
include_directories(src)
include_directories(src/coroutine)

add_executable(Z_RPC
        src/coroutine/coroutine.h)
