/**
  ******************************************************************************
  * @file           : io_thread.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-17
  ******************************************************************************
  */

#include "src/network/tcp/io_thread.h"
#include "src/common/macro.h"
#include "src/coroutine/coroutine_pool.h"
#include <unistd.h>
#include <sys/types.h>
#include <random>

namespace zrpc {

    extern std::shared_ptr<Config> zRpcConfig;
    extern std::shared_ptr<CoroutinePool> zRpcCoroutinePool;

    static thread_local Reactor* t_reactor_ptr = nullptr;

    static thread_local IOThread* t_cur_io_thread = nullptr;

    IOThread::IOThread() {
        int rt = sem_init(&m_init_semaphore, 0, 0);
        ZRPC_ASSERT(rt == 0);

        rt = sem_init(&m_start_semaphore, 0, 0);
        ZRPC_ASSERT(rt == 0);

        pthread_create(&m_thread, nullptr, &IOThread::main, this);

        DebugLog << "semaphore begin to wait until new thread finish IOThread::main() to init";
        /* wait until new thread finish IOThread::main() func to init */
        rt = sem_wait(&m_init_semaphore);
        ZRPC_ASSERT(rt == 0);
        DebugLog << "semaphore wait end, finish create io thread";

        sem_destroy(&m_init_semaphore);
    }

    IOThread::~IOThread() {
        m_reactor->stop();
        pthread_join(m_thread, nullptr);

        if (m_reactor != nullptr) {
            delete m_reactor;
            m_reactor = nullptr;
        }
    }

    Reactor* IOThread::getReactor() {
        return m_reactor;
    }

    void IOThread::addClient(TcpConnection* tcp_conn) {
        tcp_conn->registerToTimeWheel();
        tcp_conn->setUpServer();
        return;
    }

    pthread_t IOThread::getPthreadId() const {
        return m_thread;
    }

    void IOThread::setThreadIndex(int index){
        m_index = index;
    }

    int IOThread::getThreadIndex() const {
        return m_index;
    }

    sem_t* IOThread::getStartSemaphore() {
        return &m_start_semaphore;
    }

    IOThread* IOThread::GetCurrentIOThread() {
        return t_cur_io_thread;
    }

    void* IOThread::main(void* arg) {
        t_reactor_ptr = new Reactor();
        ZRPC_ASSERT(t_reactor_ptr != nullptr);
        auto io_thread = static_cast<IOThread*>(arg);
        t_cur_io_thread = io_thread;
        io_thread->m_reactor = t_reactor_ptr;
        io_thread->m_reactor->setReactorType(SubReactor);
        io_thread->m_tid = gettid();

        /* init main coroutine in this thread */
        Coroutine::InitMainCoroutine();

        DebugLog << "finish IO thread init, now post semaphore";
        sem_post(&io_thread->m_init_semaphore);

        /* wait for main thread post m_start_semaphore to start io thread loop */
        sem_wait(&io_thread->m_start_semaphore);
        sem_destroy(&io_thread->m_start_semaphore);

        DebugLog << "IOThread " << io_thread->m_tid << " begin to loop";
        t_reactor_ptr->loop();

        return nullptr;
    }

    IOThreadPool::IOThreadPool(int size) : m_size(size) {
        m_io_threads.resize(size);
        for (int i = 0; i < size; ++i) {
            m_io_threads[i] = std::make_shared<IOThread>();
            m_io_threads[i]->setThreadIndex(i);
        }
    }

    void IOThreadPool::start() {
        for(int i = 0; i < m_size; i++) {
            int rt = sem_post(m_io_threads[i]->getStartSemaphore());
            ZRPC_ASSERT(rt == 0);
        }
    }

    IOThread* IOThreadPool::getIOThread() {
        if (m_index == m_size || m_index == -1) {
            m_index = 0;
        }
        return m_io_threads[m_index++].get();
    }

    int IOThreadPool::getIOThreadPoolSize() const {
        return m_size;
    }

    void IOThreadPool::broadcastTask(std::function<void()> cb) {
        for (const auto& i : m_io_threads) {
            i->getReactor()->addTask(cb, true);
        }
    }

    void IOThreadPool::addTaskByIndex(int index, std::function<void()> cb) {
        if (index >= 0 && index < m_size) {
            m_io_threads[index]->getReactor()->addTask(cb, true);
        }
    }

    void IOThreadPool::addCoroutineToRandomThread(Coroutine::ptr cor, bool self) {
        /* if only one io thread in IOThreadPool */
        if(m_size == 1) {
            m_io_threads[0]->getReactor()->addCoroutine(cor, true);
            return;
        }

        std::default_random_engine e;
        std::uniform_int_distribution<int> num(0, m_size-1); // 闭区间
        int i = 0;
        while (true) {
            i = num(e);
            if (!self && m_io_threads[i]->getPthreadId() == t_cur_io_thread->getPthreadId()) {
                i++;
                if (i == m_size) {
                    i -= 2;
                }
            }
            break;
        }
        m_io_threads[i]->getReactor()->addCoroutine(cor, true);
    }

    Coroutine::ptr IOThreadPool::addCoroutineToRandomThread(std::function<void()> cb, bool self) {
        Coroutine::ptr cor = zRpcCoroutinePool->getCoroutineInstance();
        cor->resetCallBack(std::move(cb));
        addCoroutineToRandomThread(cor, self);
        return cor;
    }

    Coroutine::ptr IOThreadPool::addCoroutineToThreadByIndex(int index, std::function<void()> cb, bool self) {
        if (index >= (int)m_io_threads.size() || index < 0) {
            ErrorLog << "addCoroutineToThreadByIndex error, invalid io thread index[" << index << "]";
            return nullptr;
        }
        Coroutine::ptr cor = zRpcCoroutinePool->getCoroutineInstance();
        cor->resetCallBack(std::move(cb));
        m_io_threads[index]->getReactor()->addCoroutine(cor, true);
        return cor;
    }

    void IOThreadPool::addCoroutineToEachThread(std::function<void()> cb) {
        for (auto i : m_io_threads) {
            Coroutine::ptr cor =  zRpcCoroutinePool->getCoroutineInstance();
            cor->resetCallBack(cb);
            i->getReactor()->addCoroutine(cor, true);
        }
    }

}