/**
  ******************************************************************************
  * @file           : macro.hpp
  * @author         : zgys
  * @brief          : 常用宏的封装
  * @attention      : None
  * @date           : 23-3-8
  ******************************************************************************
  */


#ifndef Z_RPC_MACRO_HPP
#define Z_RPC_MACRO_HPP

#include <string>
#include <assert.h>
#include <execinfo.h>
#include <cxxabi.h>
#include "log.h"

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
    void Backtrace(std::vector<std::string> &bt, int size = 64, int skip = 1) {
        void **array = (void **) malloc((sizeof(void *) * size));
        size_t s = ::backtrace(array, size);

        char **strings = backtrace_symbols(array, s);
        if (strings == NULL) {
            ErrorLog << "backtrace_symbols error";
            return;
        }

        for (size_t i = skip; i < s; ++i) {
            bt.push_back(abi::__cxa_demangle(strings[i], nullptr, nullptr, nullptr));
        }

        free(strings);
        free(array);
    }

/**
 * @brief 获取当前栈信息的字符串
 * @param[in] size 栈的最大层数
 * @param[in] skip 跳过栈顶的层数
 * @param[in] prefix 栈信息前输出的内容
 */
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string &prefix = "") {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for (size_t i = 0; i < bt.size(); ++i) {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }
}

#endif //Z_RPC_MACRO_HPP
