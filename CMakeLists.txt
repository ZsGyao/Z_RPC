cmake_minimum_required(VERSION 3.22)
project(Z_RPC)

set(CMAKE_C_STANDARD 11)

# 指定编译选项
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++11 -O0 -ggdb -Wall -Werror")

# -Wno-unused-function: 不要警告未使用函数
# -Wno-builtin-macro-redefined: 不要警告内置宏重定义，用于重定义内置的__FILE__宏
# -Wno-deprecated: 不要警告过时的特性
# -Wno-deprecated-declarations: 不要警告使用带deprecated属性的变量，类型，函数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated -Wno-deprecated-declarations")

include_directories(.)
include_directories(src)
include_directories(src/coroutine)

set(LIB_SRC
        src/common/log.cpp
        src/common/noncopyable.hpp
        src/coroutine/coroutine.cpp
        src/network/mutex.hpp)


