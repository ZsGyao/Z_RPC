/**
  ******************************************************************************
  * @file           : macro.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-14
  ******************************************************************************
  */

#include "src/common/macro.h"

namespace zrpc {
    void Backtrace(std::vector<std::string> &bt, int size, int skip) {
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

    std::string BacktraceToString(int size, int skip, const std::string &prefix) {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for (size_t i = 0; i < bt.size(); ++i) {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }
}