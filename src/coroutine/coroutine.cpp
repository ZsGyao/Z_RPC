/**
  ******************************************************************************
  * @file           : coroutine.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-1
  ******************************************************************************
  */

#include "src/coroutine/coroutine.h"
#include <atomic>
#include <iostream>
#include "src/common/log.h"
#include "src/common/config.h"
#include "src/common/macro.h"

namespace zrpc {
    /// 静态线程局部变量， 每个线程独立拥有一份， 保存当前线程主协程
    static thread_local Coroutine::ptr t_main_coroutine = nullptr;

    /// 当前线程正在运行的协程
    static thread_local Coroutine* t_current_coroutine = nullptr;

    static thread_local RunTime* t_current_run_time = nullptr;

    /// 全局静态变量，用于统计当前协程数量
    static std::atomic<uint64_t> s_coroutine_count{0};

    /// 全局静态变量，用于生成当前协程id
    static std::atomic<uint64_t> s_current_coroutine_id{0};


    /**
     * @brief malloc栈内存分配器
     */
    class MallocStackAllocator {
    public:
        static void* Alloc(size_t size) { return malloc(size); }

        static void Dealloc(void* vp, size_t size) { return free(vp); }
    };

    using StackAllocator = MallocStackAllocator;

    // extern std::shared_ptr<Config> zRpcConfig;

    Coroutine::Coroutine() {
        t_current_coroutine = this;                         // 此时当前协程就是主协程
        m_cor_state         = RUNNING;
        m_cor_id            = s_current_coroutine_id++;     // 主协程 id = 0 , 赋值后 +1
        s_coroutine_count++;                                // 协程总数加一
        if(getcontext(&m_cor_ctx)) {                    // 将当前上下文保存到 m_cor_ctx
            ZRPC_ASSERT2(false, "getcontext");
        }

      //  std::cout << "**** Debug [coroutine.cpp " << __LINE__ <<"] Main Coroutine construct [cor_id:"<< m_cor_id << "] ****" << std::endl;
        DebugLog << "Main coroutine [ id:" << m_cor_id << " ] create";

    }

    Coroutine::Coroutine(char* stack_ptr, size_t stack_size)
             : m_cor_id(s_current_coroutine_id++),
               m_cor_stack_size(stack_size),
               m_cor_stack_ptr((char*)stack_ptr) {
        m_create_in_memory_pool = true;
        s_coroutine_count++;

        if(getcontext(&m_cor_ctx)) { // 将ucp初始化并保存当前的上下文。
            ZRPC_ASSERT2(false, "getcontext");
        }

        m_cor_ctx.uc_link          = nullptr;          // uc_link指向一个上下文，当当前上下文结束时，将返回执行该上下文。
        m_cor_ctx.uc_stack.ss_sp   = m_cor_stack_ptr;  // ss_sp栈空间的指针，指向当前栈所在的位置。
        m_cor_ctx.uc_stack.ss_size = m_cor_stack_size; // 整个栈的大小
        // 对上下文进行处理，可以创建一个新的上下文
        makecontext(&m_cor_ctx, &Coroutine::MainFunc, 0);

        //     std::cout << "debug reach cor construct2" << std::endl;
        DebugLog << "coroutine [ id:" << m_cor_id << " ] in Coroutine Pool create, stack in Memory Pool";
    }

    Coroutine::Coroutine(std::function<void()> cb, size_t stack_size)
            : m_cor_id(s_current_coroutine_id++),
              m_cor_cb(std::move(cb)) {
    //    std::cout << "debug reach cor construct1" << std::endl;
        m_create_in_memory_pool = false;

        s_coroutine_count++;
        // TODO 配置中的参数加入 test
        // std::cout << "debug reach cor construct______" << std::endl;

        //m_cor_stack_size = stack_size ? stack_size : zRpcConfig->m_cor_stack_size; // 是否选用配置
        // TODO 测试 配置中的参数是否生效 test？
        m_cor_stack_size = stack_size;
        m_cor_stack_ptr  = StackAllocator::Alloc(m_cor_stack_size);

        if(getcontext(&m_cor_ctx)) { // 将ucp初始化并保存当前的上下文。
            ZRPC_ASSERT2(false, "getcontext");
        }

        m_cor_ctx.uc_link          = nullptr;          // uc_link指向一个上下文，当当前上下文结束时，将返回执行该上下文。
        m_cor_ctx.uc_stack.ss_sp   = m_cor_stack_ptr;  // ss_sp栈空间的指针，指向当前栈所在的位置。
        m_cor_ctx.uc_stack.ss_size = m_cor_stack_size; // 整个栈的大小
        // 对上下文进行处理，可以创建一个新的上下文
        makecontext(&m_cor_ctx, &Coroutine::MainFunc, 0);

   //     std::cout << "debug reach cor construct2" << std::endl;
        DebugLog << "coroutine [ id:" << m_cor_id << " ] create, stack in Heap";
    }

    Coroutine::Coroutine(std::function<void()> cb, RunTime* run_time, size_t stack_size)
            : m_cor_id(s_current_coroutine_id++),
              m_cor_cb(std::move(cb)),
              m_cor_run_time(run_time){
        m_create_in_memory_pool = false;
        s_coroutine_count++;
        m_cor_stack_size = stack_size;
        m_cor_stack_ptr  = StackAllocator::Alloc(m_cor_stack_size);

        if(getcontext(&m_cor_ctx)) {
            ZRPC_ASSERT2(false, "getcontext");
        }

        m_cor_ctx.uc_link          = nullptr;
        m_cor_ctx.uc_stack.ss_sp   = m_cor_stack_ptr;
        m_cor_ctx.uc_stack.ss_size = m_cor_stack_size;

        makecontext(&m_cor_ctx, &Coroutine::MainFunc, 0);

    //    DebugLog << "coroutine [ id:" << m_cor_id << " ] create, stack in Heap";
    }

    /* 线程的主协程析构时需要特殊处理，因为主协程没有分配栈和cb */
    Coroutine::~Coroutine() {
        s_coroutine_count--;
        if(m_cor_stack_ptr) {
            // 有栈，说明是子协程，需要确保子协程一定是结束状态, 如果在内存池创建，不需要free
            if(!m_create_in_memory_pool) {
                // 用户创建在协程池外的，必须绑定回调，肯定是TERM状态
                ZRPC_ASSERT(m_cor_state == TERM);
                StackAllocator::Dealloc(m_cor_stack_ptr, m_cor_stack_size);
                DebugLog << "Coroutine::~Coroutine [corId:" << m_cor_id << "] in Heap destroy";
            } else {
                // 创建在协程池中的，可以没有使用过。所以READY状态就可以析构
                 ZRPC_ASSERT(m_cor_state == TERM || m_cor_state == READY);
                // free任务交给协程池去做
                // ToDo 析构在MemoryPool中创建的协程

                DebugLog << "Coroutine::~Coroutine [corId:" << m_cor_id << "] in Coroutine Pool destroy";
            }
        } else {
            // 没有栈，说明是线程的主协程
            ZRPC_ASSERT(!m_cor_cb);               // 主协程没有cb
            ZRPC_ASSERT(m_cor_state == RUNNING)   // 主协程一定是执行状态

            Coroutine* cur = t_current_coroutine; // 当前协程就是自己
            if(cur == this) {
                t_main_coroutine = nullptr;
            }
            DebugLog << "Coroutine::~Coroutine [corId:" << m_cor_id << "] Main coroutine destroy";
        }

        // std::cout << "**** Debug [coroutine.cpp 161] ~Coroutine [corId:"<< m_cor_id <<"] ****" << std::endl;
    //    DebugLog << "~coroutine [ id:" << m_cor_id << " ] destroy  [ total:" << s_coroutine_count << " ]";
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

    uint64_t Coroutine::getPoolIndex() const {
        if(m_create_in_memory_pool){
            return m_cor_pool_index;
        }
        return -1;
    }

    void Coroutine::setPoolIndex(uint64_t index) {
        if(m_create_in_memory_pool) {
            m_cor_pool_index = index;
        } else {
            ZRPC_ASSERT2(false, "setPoolIndex, this coroutine is not create in Memory Pool");
        }
    }

    bool Coroutine::IsMainCoroutine() {
        if(t_main_coroutine == nullptr || t_main_coroutine.get() == t_current_coroutine) {
            return true;
        } else {
            return false;
        }
    }

    /* 协程运行完之后会自动yield一次，用于回到主协程，此时状态已为结束状态 */
    void Coroutine::yield() {
        ZRPC_ASSERT(m_cor_state == RUNNING || m_cor_state == TERM);
        t_current_coroutine = t_main_coroutine.get();

        t_current_run_time = nullptr;

        if(m_cor_state != TERM) { // 子协程没执行完，还可以回来继续执行
            m_cor_state = READY;
        }

        if(swapcontext(&m_cor_ctx, &(t_main_coroutine->m_cor_ctx))) {
            ZRPC_ASSERT2(false, "swapcontext");
        }
    }

    void Coroutine::resume() {
       // std::cout << "reach resume" << std::endl;

        ZRPC_ASSERT(m_cor_state != RUNNING && m_cor_state != TERM); // 当前实例还是子协程，处于READY状态
        t_current_coroutine = this;

        t_current_run_time  = this->getRunTime();

        m_cor_state = RUNNING;

      //  std::cout << "reach resume______" << std::endl;

        if(swapcontext(&(t_main_coroutine->m_cor_ctx), &m_cor_ctx)) {
            ZRPC_ASSERT2(false, "swapcontext");
        }

      //  std::cout << "reach resume******" << std::endl;
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
            WarnLog << "main coroutine have already init" << std::endl;
            // WarnLog << "main coroutine have already init";
            // ZRPC_ASSERT2(false, "main coroutine have already init");
            return;
        }

        Coroutine::ptr main_cor(new Coroutine());          // 初始化调用私有构造，创建主协程
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

    RunTime* Coroutine::GetCurrentRunTime() {
        return t_current_run_time;
    }

    uint64_t Coroutine::TotalCoroutines() {
        return s_coroutine_count;
    }

    uint64_t Coroutine::GetCurrentCoroutineId() {
        return t_current_coroutine->getCorId();
    }
}