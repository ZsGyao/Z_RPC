/**
  ******************************************************************************
  * @file           : coroutine.h
  * @author         : zgys
  * @brief          : 协程
  * @attention      : None
  * @date           : 23-3-1
  ******************************************************************************
  */

#ifndef Z_RPC_COROUTINE_H
#define Z_RPC_COROUTINE_H

#include <memory>
#include <functional>
#include <ucontext.h>
#include "src/common/config.h"
#include "src/common/run_time.h"


namespace zrpc {

    extern std::shared_ptr<Config> zRpcConfig;

    void setCurrentRunTime(RunTime* v);

    /**
     * @brief 协程类
     */
    class Coroutine : public std::enable_shared_from_this<Coroutine> {
    public:
        typedef std::shared_ptr<Coroutine> ptr;

        /**
         * @brief 协程状态
         * @details 定义三态转换关系，也就是协程要么正在运行(RUNNING)，要么准备运行(READY)，要么运行结束(TERM)。
         *          不区分协程的初始状态，初始即READY。不区分协程是异常结束还是正常结束，
         *          只要结束就是TERM状态。也不区别HOLD状态，协程只要未结束也非运行态，那就是READY状态。
         */
        enum State {
            /// 就绪态，刚创建或者yield之后的状态
            READY,
            /// 运行态，resume之后的状态
            RUNNING,
            /// 结束态，协程的回调函数执行完之后为TERM状态
            TERM
        };

    private:
        /**
         * @brief 私有构造
         * @attention 无参构造函数只用于创建线程的第一个协程，也就是线程主函数对应的协程，
         *            这个协程只能由GetMainCoroutine()方法调用，所以定义成私有方法
         */
        Coroutine();

    public:
        /**
         * @brief 构造
         * @param stack_size
         * @param stack_ptr
         * @attention 在使用协程池时，可以在内存池中分配内存，传入内存池的栈指针
         */
        explicit Coroutine(char* stack_ptr, size_t stack_size = zRpcConfig->m_cor_stack_size);

        /**
         * @brief 协程构造, 用于创建用户协程
         * @param[in] stack_size 协程栈大小
         * @param[in] cb 协程执行的回调函数
         * @attention 不使用协程池，协程栈直接从堆中分配
         */
        explicit Coroutine( std::function<void()> cb, size_t stack_size = zRpcConfig->m_cor_stack_size);

        /**
         * @brief 协程构造, 用于创建用户协程
         * @param[in] stack_size 协程栈大小
         * @param[in] run_time
         * @param[in] cb 协程执行的回调函数
         * @attention 不使用协程池，协程栈直接从堆中分配
         */
        explicit Coroutine( std::function<void()> cb, RunTime* run_time, size_t stack_size = zRpcConfig->m_cor_stack_size);

        /**
         * @brief 析构函数
         */
        ~Coroutine();

        /**
         * @brief 挂起协程，当前协程让出执行权
         * @details 当前协程与上次resume时退到后台的协程进行交换，前者状态变为READY，后者状态变为RUNNING
         */
        void yield();

        /**
         * @brief 唤醒传入协程，将当前协程切到到执行状态
         * @details 当前协程和正在运行的协程进行交换，前者状态变为RUNNING，后者状态变为READY
         * @param[in] cor
         */
        void resume();


        /**
         * @brief 重置协程回调函数, 复用栈空间，不重新创建栈
         * @param[in] cb 协程回调函数
         */
        void resetCallBack(std::function<void()> cb);

        /**
         * @brief 获取协程id
         * @return 协程id
         */
        uint64_t getCorId() const {
            return m_cor_id;
        }

       /**
        * @brief 获取协程状态
        */
        State getState() const {
            return m_cor_state;
        }

        RunTime* getRunTime() const {
            return m_cor_run_time;
        }

        uint64_t getPoolIndex() const;

        void setPoolIndex(uint64_t index);

        void* getStackPtr() const {
            return m_cor_stack_ptr;
        }

    public:
        /**
         * @brief 协程入口函数
         */
        static void MainFunc();

        /**
         * @brief 初始化主协程
         */
        static void InitMainCoroutine();

        /**
         * @brief 获取当前线程正在运行的协程
         * @return 当前线程正在运行的协程
         */
        static Coroutine::ptr GetCurrentCoroutine();

        /**
         * @brief 获取当前线程的主协程
         * @return 当前线程的主协程
         */
        static Coroutine* GetMainCoroutine();

        static RunTime* GetCurrentRunTime();

        /**
         * @brief 获取总协程数
         */
        static uint64_t TotalCoroutines();

        /**
         * @brief 获取当前正在运行的协程id
         * @return
         */
        static uint64_t GetCurrentCoroutineId();

        /**
         * @brief 判断当前协程是否是主协程
         * @return true  false
         */
        static bool IsMainCoroutine();

    private:
        uint64_t              m_cor_id          = 0;                 // 协程id
        uint32_t              m_cor_stack_size  = 0;                 // 协程栈大小
        State                 m_cor_state       = READY;             // 协程状态
        ucontext_t            m_cor_ctx;                             // 协程上下文
        void*                 m_cor_stack_ptr   = nullptr;           // 协程栈地址
        std::function<void()> m_cor_cb;                              // 协程调度函数

        bool                  m_create_in_memory_pool;               // 栈内存是否从内存池中分配
        bool                  m_cor_need_resume = false;                     // 判断协程是否还需要被唤醒
        uint64_t              m_cor_pool_index      = -1;            // index in coroutine pool

        RunTime*              m_cor_run_time;
        std::string           m_cor_msg_no;

    };
}

#endif //Z_RPC_COROUTINE_H
