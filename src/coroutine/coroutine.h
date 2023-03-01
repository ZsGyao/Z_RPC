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

namespace zrpc {

    /**
     * @brief 获取协程id
     * @return
     */
    int getCoroutineIndex();

    /**
     * @brief 协程类
     */
    class Coroutine {
    public:
        typedef std::shared_ptr<Coroutine> ptr;

    private:
        /**
         * @brief 私有构造
         */
        Coroutine();

    public:
        /**
         * @brief 协程构造
         * @param[in] stack_size 协程栈大小
         * @param[in] cb 协程回调
         */
        Coroutine(size_t stack_size, std::function<void()> cb);

        /**
         * @brief 析构函数
         */
        ~Coroutine()

        /**
         * @brief 设置协程回调函数
         * @param[in] cb 协程回调函数
         * @return true:设置成功  false：设置失败
         */
        bool setCallBack(std::function<void()> cb);

        /**
         * @brief 返回协程id
         * @return 协程id
         */
        uint64_t getCorId() const {
            return m_cor_id;
        }

        /**
         * @brief 设置是否正在执行此协程的回调
         * @param[in] v true or false
         */
        void setIsRunCorFunc(const bool v) {
            m_is_run_cor_func = v;
        }

        /**
         * @brief 获取是否正在执行此协程的回调
         * @return true: 正在执行  false：不在执行
         */
        bool getIsRunCorFunc() const {
            return m_is_run_cor_func;
        }

        /**
         * @brief 设置是否可以唤醒协程
         * @param[in] v true or false
         */
        void setCanResume(bool v) {
            m_cor_can_resume = v;
        }

    public:
        /**
         * @brief 挂起协程
         */
        static void Yield();

        /**
         * @brief 唤醒传入协程
         * @param[in] cor
         */
        static void Resume(Coroutine* cor);

        /**
         * @brief 获取当前线程正在运行的协程
         * @return 当前线程正在运行的协程
         */
        static Coroutine* GetCurrentCoroutine();

        /**
         * @brief 获取当前线程的主协程
         * @return 当前线程的主协程
         */
        static Coroutine* GetMainCoroutine();

        /**
         * @brief 判断当前协程是否是主协程
         * @return true  false
         */
        static bool IsMainCoroutine();

    private:
        uint64_t              m_cor_id = 0;                 // 协程id
        uint32_t              m_cor_stack_size = 0;         // 协程栈大小
        ucontext_t            m_cor_ctx;                    // 协程上下文
        void*                 m_cor_stack_ptr = nullptr;    // 协程栈地址
        std::function<void()> m_cor_cb                      // 协程调度函数
        bool                  m_is_run_cor_func = false;    // 是否正在执行此协程的回调
        bool                  m_cor_can_resume = true;      // 是否可以唤醒协程
    };
}

#endif //Z_RPC_COROUTINE_H
