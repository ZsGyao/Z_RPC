/**
  ******************************************************************************
  * @file           : io_thread.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-17
  ******************************************************************************
  */

#ifndef ZRPC_IO_THREAD_H
#define ZRPC_IO_THREAD_H

#include <memory>
#include <semaphore.h>
#include "src/network/reactor.h"
#include "src/network/tcp/tcp_connect_time_wheel.h"

namespace zrpc {

    extern std::shared_ptr<Config> zRpcConfig;

    class IOThread {
    public:
        typedef std::shared_ptr<IOThread> ptr;

        IOThread();

        ~IOThread();

        Reactor* getReactor();

        void addClient(TcpConnection* tcp_conn);

        pthread_t getPthreadId() const;

        void setThreadIndex(int index);

        int getThreadIndex() const;

        sem_t* getStartSemaphore();

    public:
        static IOThread* GetCurrentIOThread();

    private:
        static void* main(void* arg);

    private:
        Reactor*  m_reactor = nullptr; // the reactor create in thread that pthread_create,  initialize in IOThread::main
        pthread_t m_thread  = 0;       // which thread to execute IOThread::main
        pid_t     m_tid     = -1;
        int       m_index   = -1;
        TimerEvent::ptr m_timer_event = nullptr;

        sem_t m_init_semaphore;
        sem_t m_start_semaphore;
    };

    class IOThreadPool {
    public:
        typedef std::shared_ptr<IOThreadPool> ptr;

        IOThreadPool(int size = zRpcConfig->m_io_thread_num);

        /* wakeup the io_thread that block in IOThread::main */
        void start();

        IOThread* getIOThread();

        int getIOThreadPoolSize() const;

        /* register the task cb to the reactor in all the io thread */
        void broadcastTask(std::function<void()> cb);

        /* register the task to the io thread which is vector m_io_threads[index] */
        void addTaskByIndex(int index, std::function<void()> cb);

        void addCoroutineToRandomThread(Coroutine::ptr cor, bool self = false);

        /* add a coroutine to random thread in io thread pool
           self = false, means random thread can not be current thread
           self = true, means this cor task must be registered into current thread reactor
           please free cor, or causes memory leak
           call returnCoroutine(cor) to free coroutine */
        Coroutine::ptr addCoroutineToRandomThread(std::function<void()> cb, bool self = false);

        Coroutine::ptr addCoroutineToThreadByIndex(int index, std::function<void()> cb, bool self = false);

        /* put the task to coroutine and register cor to reactor */
        void addCoroutineToEachThread(std::function<void()> cb);

    private:
        int m_size = 0;
        std::atomic<int> m_index {-1};
        std::vector<IOThread::ptr> m_io_threads; // the vector that manager io thread in IOThreadPool
    };
}

#endif //ZRPC_IO_THREAD_H
