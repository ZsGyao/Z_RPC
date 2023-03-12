/**
  ******************************************************************************
  * @file           : thread_pool.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-11
  ******************************************************************************
  */


#ifndef Z_RPC_THREAD_POOL_H
#define Z_RPC_THREAD_POOL_H

#include <pthread.h>
#include <functional>
#include <vector>
#include "src/network/mutex.hpp"

namespace zrpc {

    class ThreadPool {
    public:
        typedef Mutex MutexType;

        ThreadPool(int size);

        ~ThreadPool();

        void start();

        void stop();

        void addTask(std::function<void()> cb);

    private:
        static void* MainFunction(void* ptr);


    public:
        int                               m_size = 0;
        std::vector<pthread_t>            m_threads;
        std::queue<std::function<void()>> m_tasks;

        MutexType                             m_mutex;
        pthread_cond_t                    m_condition;
        bool                              m_is_stop = false;
    };
}

#endif //Z_RPC_THREAD_POOL_H
