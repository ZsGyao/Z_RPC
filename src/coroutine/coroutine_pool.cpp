/**
  ******************************************************************************
  * @file           : coroutine_pool.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-11
  ******************************************************************************
  */

#include "src/coroutine/coroutine_pool.h"
#include "src/common/config.h"
#include "src/common/log.h"

namespace zrpc {

    extern std::shared_ptr<Config> zRpcConfig;

    std::shared_ptr<zrpc::CoroutinePool> zRpcCoroutinePool = Singleton_CoroutinePool::GetInstance(zRpcConfig->m_cor_pool_size,
                                                                  zRpcConfig->m_cor_stack_size);

    CoroutinePool::CoroutinePool(int pool_size, int stack_size)
            : m_cor_pool_size(pool_size),
              m_stack_size(stack_size) {
        // init main coroutine
        Coroutine::InitMainCoroutine();

        /* 分配一个协程池所有协程栈大小的内存池，由 m_memory_pool管理 */
        m_memory_pool.push_back(std::make_shared<MemoryPool>(stack_size, pool_size));

        MemoryPool::ptr mem_tmp = m_memory_pool[0];

        for(int i = 0; i < m_cor_pool_size; i++) {
            Coroutine::ptr cor = std::make_shared<Coroutine>(mem_tmp->getBlock(), m_stack_size);
            cor->setPoolIndex(i);
            m_free_cors.emplace_back(std::make_pair(cor,false));
        }
        DebugLog << "CoroutinePool create : coroutine number [" << m_cor_pool_size <<"]  stack size [" << m_stack_size / 1024 << " KB" << "]";
    }

    CoroutinePool::~CoroutinePool() {
        DebugLog << "~CoroutinePool destroy";
    }

    /* try our best to reuse used coroutine, and try our best not to choose unused coroutine
     * because used coroutine which used has already written bytes into physical memory,
     * but unused coroutine no physical memory yet. we just call mmap get virtual address, but not write yet.
     * so linux will alloc physical when we ready write, that cause page fault interrupt
     */
    Coroutine::ptr CoroutinePool::getCoroutineInstance() {
        MutexType::Lock lock(m_mutex);

        for(auto& cor : m_free_cors) {
            // 协程状态是TERM，并且可以 dispatch
            if(cor.first->getState() == Coroutine::TERM && !cor.second) {
                cor.second = true;
                Coroutine::ptr free_cor = cor.first;
                lock.unlock();
                return free_cor;
            }
        }

        for(const auto& i : m_memory_pool) {
            char* tmp = i->getBlock();
            if(tmp) {
                Coroutine::ptr cor = std::make_shared<Coroutine>(tmp, m_stack_size);
                return cor;
            }
        }

        m_memory_pool.emplace_back(std::make_shared<MemoryPool>(m_stack_size, m_cor_pool_size));
        return std::make_shared<Coroutine>(m_memory_pool[m_memory_pool.size() - 1]->getBlock(), m_stack_size);
    }

    void CoroutinePool::returnCoroutine(Coroutine::ptr cor) {
        int index = cor->getPoolIndex();
        if(index >= 0 && index < m_cor_pool_size) {
            m_free_cors[index].second = false;
        } else {
            for(const auto& i : m_memory_pool) {
                if(i->hasBlock((char*)cor->getStackPtr())) {
                    i->backBlock((char*)cor->getStackPtr());
                }
            }
        }
    }
}