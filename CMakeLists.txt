cmake_minimum_required(VERSION 3.22)
project(zRPC)

set(CMAKE_C_STANDARD 11)

include (cmake/utils.cmake)

# -rdynamic: 将所有符号都加入到符号表中，便于使用dlopen或者backtrace追踪到符号
# -fPIC: 生成位置无关的代码，便于动态链接
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -fPIC")

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
include_directories(src/common)
include_directories(src/network)

option(BUILD_TEST "ON for complile test" ON)

set(LIB_SRC
        src/common/log.cpp
        src/common/macro.cpp
        src/common/config.cpp
        src/common/noncopyable.hpp
        src/common/singleton.hpp
        src/common/start.cpp
        src/common/thread_pool.cpp
        src/common/string_util.cpp
        src/coroutine/coroutine.cpp
        src/coroutine/hook.cpp
        src/coroutine/coroutine_pool.cpp
        src/coroutine/memory_pool.cpp
        src/network/mutex.hpp
        src/network/byte.hpp
        src/network/fd_event.cpp
        src/network/timer.cpp
        src/network/net_address.cpp
        src/network/reactor.cpp
        src/network/tcp/tcp_connect_timer_wheel.cpp
        src/network/tcp/tcp_buffer.cpp
        src/network/tcp/io_thread.cpp
        src/network/http/http_define.cpp
        src/network/http/http_codec.cpp
        src/network/http/http_servlet.h src/network/http/http_servlet.cpp src/network/http/http_dispatcher.h src/network/abstract_dispatcher.h src/network/tcp/tcp_connection.h src/network/zRpcPb/zrpc_pb_codec.h src/network/zRpcPb/zrpc_pb_data.h)


add_library(src SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(src)

set(LIBS
        src
        tinyxml2
        )

add_executable(test_coroutine tests/test_coroutine.cpp)
add_dependencies(test_coroutine src)
force_redefine_file_macro_for_sources(test_coroutine)  #__FILE__
target_link_libraries(test_coroutine ${LIBS})

add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log src)
force_redefine_file_macro_for_sources(test_log)  #__FILE__
target_link_libraries(test_log ${LIBS})

add_executable(test_macro tests/test_macro.cpp)
add_dependencies(test_macro src)
force_redefine_file_macro_for_sources(test_macro)  #__FILE__
target_link_libraries(test_macro ${LIBS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
