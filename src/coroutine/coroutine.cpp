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
#include "common/log.h"
#include "common/macro.hpp"
#include "common/config.h"

namespace zrpc {
    /// 静态线程局部变量， 每个线程独立拥有一份， 保存当前线程主协程
    static thread_local Coroutine::ptr t_main_coroutine = nullptr;

    /// 当前线程正在运行的协程
    static thread_local Coroutine* t_current_coroutine = nullptr;

    /// 全局静态变量，用于统计当前协程数量
    static std::atomic<uint64_t> s_coroutine_count{0};

    /// 全局静态变量，用于生成当前协程id
    static std::atomic<uint64_t> s_current_coroutine_id{0};

    static Config::ptr g_config = std::make_shared<Config>("Z_RPC/config/zrpc_server.xml");

    /**
     * @brief malloc栈内存分配器
     */
    class MallocStackAllocator {
    public:
        static void* Alloc(size_t size) { return malloc(size); }

        static void Dealloc(void* vp, size_t size) { return free(vp); }
    };

    using StackAllocator = MallocStackAllocator;


    Coroutine::Coroutine() {
        t_current_coroutine = this;                         // 此时当前协程就是主协程
        m_cor_state         = RUNNING;
        if(getcontext(&m_cor_ctx)) {                    // 将当前上下文保存到 m_cor_ctx
            ZRPC_ASSERT2(false, "getcontext");
        }
        m_cor_id            = s_current_coroutine_id++;     // 主协程 id = 0 , 赋值后 +1
        s_coroutine_count++;                                // 协程总数加一

        DebugLog << "main coroutine [ id:" << m_cor_id << " ] create";
    }

    Coroutine::Coroutine(std::function<void()> cb, size_t stack_size)
            : m_cor_id(s_current_coroutine_id++),
              m_cor_cb(std::move(cb)) {
        s_coroutine_count++;
        // TODO 配置中的参数加入 test
        m_cor_stack_size = stack_size ? stack_size : g_config->m_cor_stack_size; // 是否选用配置
        m_cor_stack_ptr  = StackAllocator::Alloc(m_cor_stack_size);

        if(getcontext(&m_cor_ctx)) { // 将ucp初始化并保存当前的上下文。
            ZRPC_ASSERT2(false, "getcontext");
        }

        m_cor_ctx.uc_link          = nullptr;          // uc_link指向一个上下文，当当前上下文结束时，将返回执行该上下文。
        m_cor_ctx.uc_stack.ss_sp   = m_cor_stack_ptr;  // ss_sp栈空间的指针，指向当前栈所在的位置。
        m_cor_ctx.uc_stack.ss_size = m_cor_stack_size; // 整个栈的大小
        // 对上下文进行处理，可以创建一个新的上下文
        makecontext(&m_cor_ctx, &Coroutine::MainFunc, 0);

        DebugLog << "coroutine [ id:" << m_cor_id << " ] create";
    }

    /* 线程的主协程析构时需要特殊处理，因为主协程没有分配栈和cb */
    Coroutine::~Coroutine() {
        DebugLog << "~coroutine [ id:" << m_cor_id << " ] destroy";

        s_coroutine_count--;
        if(m_cor_stack_ptr) {
            // 有栈，说明是子协程，需要确保子协程一定是结束状态
            ZRPC_ASSERT(m_cor_state == TERM);
            StackAllocator::Dealloc(m_cor_stack_ptr, m_cor_stack_size);
            DebugLog << "dealloc stack, coroutine [ id:" << m_cor_id << " ]";
        } else {
            // 没有栈，说明是线程的主协程
            ZRPC_ASSERT(!m_cor_cb);               // 主协程没有cb
            ZRPC_ASSERT(m_cor_state == RUNNING)   // 主协程一定是执行状态

            Coroutine* cur = t_current_coroutine; // 当前协程就是自己
            if(cur == this) {
                t_main_coroutine = nullptr;
            }
        }
    }

    /* 这里为了简化状态管理，强制只有TERM状态的协程才可以重置，但其实刚创建好但没执行过的协程也应该允许重置的 */
    void Coroutine::resetCallBack(std::function<void()> cb) {
        ZRPC_ASSERT(m_cor_stack_ptr);      // 当前协程栈指针存在，说明是子协程
        ZRPC_ASSERT(m_cor_state == TERM);  // 协程在中止状态才可以重设回调

        m_cor_cb = std::move(cb);
        if(getcontext(&m_cor_ctx)) {
            ZRPC_ASSERT2(false, "getcontext");
        }

        m_cor_ctx.uc_link          = nullptr;
        m_cor_ctx.uc_stack.ss_sp   = m_cor_stack_ptr;
        m_cor_ctx.uc_stack.ss_size = m_cor_stack_size;

        makecontext(&m_cor_ctx, &Coroutine::MainFunc, 0);
        m_cor_state = READY;
    }

    bool Coroutine::isMainCoroutine() {
        if(t_main_coroutine.get() == this) {
            return true;
        } else {
            return false;
        }
    }

    /* 协程运行完之后会自动yield一次，用于回到主协程，此时状态已为结束状态 */
    void Coroutine::yield() {
        ZRPC_ASSERT(m_cor_state == RUNNING || m_cor_state == TERM);
        t_current_coroutine = t_main_coroutine.get();
        if(m_cor_state != TERM) { // 子协程没执行完，还可以回来继续执行
            m_cor_state = READY;
        }

        if(swapcontext(&m_cor_ctx, &(t_main_coroutine->m_cor_ctx))) {
            ZRPC_ASSERT2(false, "swapcontext");
        }
    }

    void Coroutine::resume() {
        ZRPC_ASSERT(m_cor_state != RUNNING && m_cor_state != TERM); // 当前实例还是子协程，处于READY状态
        t_current_coroutine = this;
        m_cor_state = RUNNING;

        if(swapcontext(&(t_main_coroutine->m_cor_ctx), &m_cor_ctx)) {
            ZRPC_ASSERT2(false, "swapcontext");
        }
    }

    void Coroutine::MainFunc() {
        Coroutine::ptr cur = t_current_coroutine->shared_from_this(); // t_current_coroutine 引用计数加1
        ZRPC_ASSERT(cur);

        cur->m_cor_cb();             // 执行协程的回调
        cur->m_cor_cb    = nullptr;  // 执行完成后置空
        cur->m_cor_state = TERM;

        auto raw_ptr = cur.get();
        cur.reset();                 // 手动让t_current_coroutine的引用计数减1
        raw_ptr->yield();
    }

    void Coroutine::InitMainCoroutine() {
        if(t_main_coroutine) {
            ZRPC_ASSERT2(false, "main coroutine have already init");
        }

        Coroutine::ptr main_cor(new Coroutine);            // 初始化调用私有构造，创建主协程
        ZRPC_ASSERT(t_current_coroutine == main_cor.get());  // 在构造主协程时已经把t_current_coroutine设置为主协程
        t_main_coroutine = main_cor;                         // 设置 t_main_coroutine
    }

    Coroutine::ptr Coroutine::GetCurrentCoroutine() {
        if(!t_main_coroutine) {
            ZRPC_ASSERT2(false, "t_main_coroutine not init, try to call InitMainCoroutine()");
        }
        return t_current_coroutine->shared_from_this();
    }

    Coroutine* Coroutine::GetMainCoroutine() {
        if(!t_main_coroutine) {
            ZRPC_ASSERT2(false, "t_main_coroutine not init, try to call InitMainCoroutine()");
        }
        return t_main_coroutine.get();
    }

    uint64_t Coroutine::TotalCoroutines() {
        return s_coroutine_count;
    }

    uint64_t Coroutine::getCurrentCoroutineId() {
        return t_current_coroutine->getCorId();
    }
}