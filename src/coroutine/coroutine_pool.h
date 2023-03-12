/**
  ******************************************************************************
  * @file           : coroutine_pool.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-11
  ******************************************************************************
  */


#ifndef Z_RPC_COROUTINE_POOL_H
#define Z_RPC_COROUTINE_POOL_H

#include <vector>
#include <memory>
#include "src/coroutine/coroutine.h"
#include "src/coroutine/memory_pool.h"
#include "src/network/mutex.hpp"
#include "src/common/singleton.hpp"

namespace zrpc {

    class CoroutinePool {
    public:
        typedef std::shared_ptr<CoroutinePool> ptr;
        typedef Mutex MutexType;

        CoroutinePool(int pool_size, int stack_size = 128 * 1024);
        ~CoroutinePool();

        Coroutine::ptr getCoroutineInstance();

        void returnCoroutine(Coroutine::ptr cor);

    private:
        int m_cor_pool_size = 0;         // 协程池中协程个数
        int m_stack_size    = 0;         // 每个协程栈空间大小

        /* first  --> coroutine ptr
         * second --> whether coroutine can be dispatched
         */
        std::vector<std::pair<Coroutine::ptr, bool>> m_free_cors;
        std::vector<MemoryPool::ptr>                 m_memory_pool;

        MutexType m_mutex;
    };

    typedef zrpc::Singleton<CoroutinePool> Singleton_CoroutinePool;
    extern std::shared_ptr<CoroutinePool> zRpcCoroutinePool;
}

#endif //Z_RPC_COROUTINE_POOL_H
