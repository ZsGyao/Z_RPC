/**
  ******************************************************************************
  * @file           : log.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-1
  ******************************************************************************
  */

#include <atomic>

namespace zrpc {
    /// RPC_LOG类型日志写入的index
    static std::atomic<int64_t> g_rpc_log_index {0};
    /// APP_LOG类型日志写入的index
    static std::atomic<int64_t> g_app_log_index {0};
}