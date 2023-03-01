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

namespace ZRPC {
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

    private:
        uint64_t m_cor_id              = 0;           // 协程id
        uint32_t m_cor_stack_size      = 0;          // 协程栈大小
        ucontext_t m_cor_ctx;                        //协程上下文
        void* m_cor_stack_ptr          = nullptr;    // 协程栈地址
        std::function<void()> m_cor_cb               // 协程调度函数
        bool m_is_run_cor_func         = false;      // 是否正在执行此协程的回调
    };
}

#endif //Z_RPC_COROUTINE_H
