/**
  ******************************************************************************
  * @file           : memory_pool.hpp
  * @author         : zgys
  * @brief          : 协程的内存池，每一个协程池分配一块 协程池中协程个数 * 协程栈大小的内存块
  * @attention      : None
  * @date           : 23-3-11
  ******************************************************************************
  */


#ifndef Z_RPC_MEMORY_POOL_HPP
#define Z_RPC_MEMORY_POOL_HPP

#include <memory>
#include <atomic>
#include <vector>
#include <src/network/mutex.hpp>

namespace zrpc {

    /**
     * @brief 内存池，每一个协程池分配一块 协程池中协程个数 * 协程栈大小的内存块
     * @attention N 协程池 --> M 内存池
     *            N 协程  --> N block
     *            1 block -> 一个协程栈的大小
     */
    class MemoryPool {
    public:
        typedef std::shared_ptr<MemoryPool> ptr;
        typedef Mutex MutexType;

        MemoryPool(int block_size, int block_count);

        ~MemoryPool();

        /**
         * @brief 获取已被使用的内存块个数
         * @return
         */
        int getRefCount();

        /**
         * @brief 获取内存池起始位置指针
         * @return 内存池起始位置指针
         */
        char* getStart();

        /**
         * @brief 获取下一块未使用过的block
         * @return 新block的起始位置指针, block已经使用完，return nullptr
         */
        char* getBlock();

        /**
         * @attention 放弃 (协程栈指针s) 所在的块
         * @param s
         */
        void backBlock(char* s);

        bool hasBlock(char* s);

    private:
        int   m_mem_block_size  = 0;               // 每个内存块大小
        int   m_mem_block_count = 0;               // 内存块个数

        int   m_mem_size        = 0;               // 内存池大小
        char* m_mem_start       = nullptr;         // 内存池起始位置
        char* m_mem_end         = nullptr;         // 内存池结束位置

        std::atomic<int>  m_ref_counts {0};      // 已经使用的内存块个数
        std::vector<bool> m_mem_blocks;           // index的块是否被使用
        MutexType         m_mutex;
    };
}

#endif //Z_RPC_MEMORY_POOL_HPP
