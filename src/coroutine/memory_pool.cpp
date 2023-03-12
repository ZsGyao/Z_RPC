/**
  ******************************************************************************
  * @file           : memory_pool.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-11
  ******************************************************************************
  */

#include "src/coroutine/memory_pool.h"
#include "src/common/macro.hpp"

namespace zrpc {

    MemoryPool::MemoryPool(int block_size, int block_count)
            : m_mem_block_size(block_size),
              m_mem_block_count(block_count) {
        m_mem_size = m_mem_block_size * m_mem_block_count;
        m_mem_start = (char*) malloc(m_mem_size);
        ZRPC_ASSERT(m_mem_start != (void*)-1);
        DebugLog << "MemoryPoll create success mmap [" << m_mem_size << " bytes] memory";

        m_mem_end = m_mem_start + m_mem_size;
        m_mem_blocks.resize(m_mem_block_count);

        for(size_t i = 0; i < m_mem_blocks.size(); i++) {
            m_mem_blocks[i] = false;
        }
        m_ref_counts = 0;
    }

    MemoryPool::~MemoryPool() {
        if (!m_mem_start || m_mem_start == (void*)-1) {
            return;
        }
        free(m_mem_start);
        DebugLog << "~MemoryPoll destroy success free [" << m_mem_size << " bytes] memory";
        m_mem_start = nullptr;
        m_ref_counts = 0;
    }

    int MemoryPool::getRefCount() {
        return m_ref_counts;
    }

    char* MemoryPool::getStart() {
        return m_mem_start;
    }

    char* MemoryPool::getBlock() {
        int used_num = -1;
        MutexType::Lock lock(m_mutex);
        for (size_t i = 0; i < m_mem_blocks.size(); ++i) {
            if (!m_mem_blocks[i]) {
                m_mem_blocks[i] = true;
                used_num = i;
                break;
            }
        }
        lock.unlock();
        if(used_num == -1) {
            return nullptr;
        }

        m_ref_counts++;
        if(hasBlock(m_mem_start + (used_num * m_mem_block_size))){
            return m_mem_start + (used_num * m_mem_block_size);
        }
        return nullptr;
    }

    void MemoryPool::backBlock(char* s) {
        if(s > m_mem_end || s < m_mem_start) {
            ErrorLog << "ERROR!! This block not belongs to this Memory Poll";
            return;
        }
        int block_index = (s - m_mem_start) / m_mem_block_size;
        MutexType::Lock lock(m_mutex);
        m_mem_blocks[block_index] = false;
        lock.unlock();
        m_ref_counts--;
    }

    bool MemoryPool::hasBlock(char* s) {
        return (s >= m_mem_start && s < m_mem_end);
    }

}