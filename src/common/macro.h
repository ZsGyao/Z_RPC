/**
  ******************************************************************************
  * @file           : macro.hpp
  * @author         : zgys
  * @brief          : 常用宏的封装
  * @attention      : None
  * @date           : 23-3-8
  ******************************************************************************
  */


#ifndef Z_RPC_MACRO_H
#define Z_RPC_MACRO_H

#include <string>
#include <assert.h>
#include <execinfo.h>
#include <cxxabi.h>
#include "src/common/log.h"


#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#define ZRPC_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#define ZRPC_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define ZRPC_LIKELY(x) (x)
#define ZRPC_UNLIKELY(x) (x)
#endif

/// 断言宏封装
#define ZRPC_ASSERT(x)                                         \
    if (ZRPC_UNLIKELY(!(x))) {                                 \
        ErrorLog << "ASSERTION: " #x                           \
                 << "\nbacktrace:\n"                           \
                 << zrpc::BacktraceToString(100, 2, "    ");   \
        assert(x);                                             \
    }

/// 断言宏封装
#define ZRPC_ASSERT2(x, w)                                    \
    if (ZRPC_UNLIKELY(!(x))) {                                \
        ErrorLog << "ASSERTION: " #x                          \
                 << "\n"                                      \
                 << w                                         \
                 << "\nbacktrace:\n"                          \
                 << zrpc::BacktraceToString(100, 2, "    ");  \
        assert(x);                                            \
    }

namespace zrpc {
    /**
     * @brief 获取当前的调用栈
     * @param[out] bt 保存调用栈
     * @param[in] size 最多返回层数
     * @param[in] skip 跳过栈顶的层数
     */
     void Backtrace(std::vector<std::string> &bt, int size = 64, int skip = 1);

/**
 * @brief 获取当前栈信息的字符串
 * @param[in] size 栈的最大层数
 * @param[in] skip 跳过栈顶的层数
 * @param[in] prefix 栈信息前输出的内容
 */
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string &prefix = "");
}

#endif //Z_RPC_MACRO_H
