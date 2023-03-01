/**
  ******************************************************************************
  * @file           : mutex.h
  * @author         : zgys
  * @brief          : 封装锁
  * @attention      : 参考 sylar
  * @date           : 23-3-1
  ******************************************************************************
  */


#ifndef Z_RPC_MUTEX_H
#define Z_RPC_MUTEX_H

#include <pthread.h>
#include <memory>
#include <queue>
#include "src/common/noncopyable.hpp"
#include "src/coroutine/coroutine.h"

namespace zrpc {

    /**
     * @brief 局部锁的模板实现
     * @tparam T 模板参数
     */
    template <class T>
    struct ScopedLockImpl {
    public:
        /**
         * @brief 构造函数
         * @param[in] mutex Mutex
         */
        ScopedLockImpl(T& mutex)
                : m_mutex(mutex) {
            m_mutex.lock();
            m_locked = true;
        }

        /**
         * @brief 析构函数,自动释放锁
         */
        ~ScopedLockImpl() {
            unlock();
        }

        /**
         * @brief 加锁
         */
        void lock() {
            if (!m_locked) {
                m_mutex.lock();
                m_locked = true;
            }
        }

        /**
         * @brief 解锁
         */
        void unlock() {
            if (m_locked) {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        /// mutex
        T &m_mutex;
        /// 是否已上锁
        bool m_locked;
    };

    /**
     * @brief 局部读锁模板实现
     */
    template <class T>
    struct ReadScopedLockImpl {
    public:
        /**
         * @brief 构造函数
         * @param[in] mutex 读写锁
         */
        ReadScopedLockImpl(T &mutex)
                : m_mutex(mutex) {
            m_mutex.rdlock();
            m_locked = true;
        }

        /**
         * @brief 析构函数,自动释放锁
         */
        ~ReadScopedLockImpl() {
            unlock();
        }

        /**
         * @brief 上读锁
         */
        void lock() {
            if (!m_locked) {
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        /**
         * @brief 释放锁
         */
        void unlock() {
            if (m_locked) {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        /// mutex
        T &m_mutex;
        /// 是否已上锁
        bool m_locked;
    };

    /**
     * @brief 局部写锁模板实现
     */
    template <class T>
    struct WriteScopedLockImpl {
    public:
        /**
         * @brief 构造函数
         * @param[in] mutex 读写锁
         */
        WriteScopedLockImpl(T &mutex)
                : m_mutex(mutex) {
            m_mutex.wrlock();
            m_locked = true;
        }

        /**
        * @brief 析构函数
        */
        ~WriteScopedLockImpl() {
            unlock();
        }

        /**
         * @brief 上写锁
         */
        void lock() {
            if (!m_locked)
            {
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        /**
         * @brief 解锁
         */
        void unlock() {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        /// Mutex
        T &m_mutex;
        /// 是否已上锁
        bool m_locked;
    };

    /**
     * @brief 互斥量
     */
    class Mutex : Noncopyable {
    public:
        /// 局部锁
        typedef ScopedLockImpl<Mutex> Lock;

        /**
         * @brief 构造函数
         */
        Mutex() {
            pthread_mutex_init(&m_mutex, nullptr);
        }

        /**
        * @brief 析构函数
        */
        ~Mutex() {
            pthread_mutex_destroy(&m_mutex);
        }

        /**
         * @brief 加锁
         */
        void lock() {
            pthread_mutex_lock(&m_mutex);
        }

        /**
         * @brief 解锁
         */
        void unlock() {
            pthread_mutex_unlock(&m_mutex);
        }

        /**
         * @brief 获取锁
         * @return
         */
        pthread_mutex_t* getMutex() {
            return& m_mutex;
        }

    private:
        /// mutex
        pthread_mutex_t m_mutex;
    };

    class RWMutex : Noncopyable {
    public:
        /// 局部读锁
        typedef ReadScopedLockImpl<RWMutex> ReadLock;

        /// 局部写锁
        typedef WriteScopedLockImpl<RWMutex> WriteLock;

        RWMutex() {
            pthread_rwlock_init(&m_lock, nullptr);
        }

        ~RWMutex() {
            pthread_rwlock_destroy(&m_lock);
        }

        /**
         * @brief 上读锁
         */
        void rdlock() {
            pthread_rwlock_rdlock(&m_lock);
        }

        /**
         * @brief 上写锁
         */
        void wrlock()
        {
            pthread_rwlock_wrlock(&m_lock);
        }

        /**
         * @brief 解锁
         */
        void unlock()
        {
            pthread_rwlock_unlock(&m_lock);
        }

    private:
        /// 读写锁
        pthread_rwlock_t m_lock;
    };


    class CoroutineMutex : Noncopyable {
    public:
        typedef ScopedLockImpl<CoroutineMutex> Lock;

        CoroutineMutex();

        ~CoroutineMutex();

        void lock();

        void unlock();

    private:
        bool                   m_lock {false};
        Mutex                  m_mutex;
        std::queue<Coroutine*> m_sleep_cors;
    };
}

#endif //Z_RPC_MUTEX_H
