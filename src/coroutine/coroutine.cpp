/**
  ******************************************************************************
  * @file           : coroutine.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-1
  ******************************************************************************
  */

#include "coroutine.h"
#include <atomic>

namespace zrpc {
    /// 静态线程局部变量， 每个线程独立拥有一份， 保存当前线程主协程
    static thread_local Coroutine* t_main_coroutine = nullptr;

    /// 当前线程正在运行的协程
    static thread_local Coroutine* t_current_coroutine = nullptr;

    /// 协程数量
    static std::atomic<uint64_t> s_coroutine_count {0};

    /// 当前协程id
    static std::atomic<uint64_t> s_current_coroutine_id {0};


    int getCoroutineIndex() {
        return s_current_coroutine_id;
    }

    Coroutine::Coroutine() {
        m_cor_id = s_current_coroutine_id++;     // 主协程 id = 0 , 赋值后 +1
        s_coroutine_count++;                     // 协程总数加一
        getcontext(&m_cor_ctx);              // 将当前上下文保存到 m_cor_ctx
        t_main_coroutine = this;                 // 设置当前线程主协程
        t_current_coroutine = this;              // 此时当前协程就是主协程
    }

    Coroutine::Coroutine(size_t stack_size, std::function<void()> cb) {

    }

    void  Coroutine::Yield() {

    }

    void Coroutine::Resume(Coroutine* cor) {

    }

    Coroutine* Coroutine::GetCurrentCoroutine() {

    }

    Coroutine* Coroutine::GetMainCoroutine() {

    }

    bool Coroutine::IsMainCoroutine() {

    }
}