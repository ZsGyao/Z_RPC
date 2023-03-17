/**
  ******************************************************************************
  * @file           : reactor.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-13
  ******************************************************************************
  */


#ifndef Z_RPC_REACTOR_H
#define Z_RPC_REACTOR_H

#include <memory>
#include <sys/epoll.h>
#include <functional>
#include <vector>
#include <atomic>
#include "src/network/mutex.hpp"
#include "src/coroutine/coroutine.h"



namespace zrpc {

    enum ReactorType {
        MainReactor = 1, // main reactor， 设置在主线程上
        SubReactor  = 2  // chile reactor， 其他所有IO
    };

    class Timer;
    class FdEvent;

    class Reactor {
    public:
        typedef std::shared_ptr<Reactor> ptr;
        typedef Mutex MutexType;

        Reactor();

        ~Reactor();

        void addEvent(int fd, epoll_event event, bool is_wakeup = true);

        void delEvent(int fd, bool is_wakeup = true);

        void addTask(const std::function<void()>& task, bool is_wakeup = true);

        void addTask(std::vector<std::function<void()>> task, bool is_wakeup = true);

        void addCoroutine(zrpc::Coroutine::ptr cor, bool is_wakeup = true);

        /* use eventfd() that write into m_wake_fd, and epoll_wait in reactor will back and execute the task */
        void wakeup() const;

        void loop();

        void stop();

        Timer* getTimer();

        pid_t getTid() const;

        void setReactorType(ReactorType type);

    public:
        static Reactor* GetReactor();

    private:
        void addWakeupFd();

        /**
         * @brief 是否是循环检测fd的线程
         * @return true or false
         */
        bool isLoopThread() const;
        /**
         * @brief 在循环线程中添加事件
         * @param fd
         * @param event
         */
        void addEventInLoopThread(int fd, epoll_event event);

        void delEventInLoopThread(int fd);

    private:
        int   m_epfd          = -1;
        int   m_wake_fd       = -1;           // wakeup fd，read_fd or write_fd
        int   m_timer_fd      = -1;           // timer fd
        bool  m_stop_flag     = false;
        bool  m_is_looping    = false;
        bool  m_is_init_timer = false;
        pid_t m_tid           = 0;            // thread id

        MutexType        m_mutex;            // mutex

        std::vector<int> m_fds;              // already care events
        std::atomic<int> m_fd_size {0};    // reactor 注册的fd数

        /* fds that wait for operate
         * 1 -- to add to loop
         * 2 -- to del from loop */
        std::map<int, epoll_event>         m_pending_add_fds;
        std::vector<int>                   m_pending_del_fds;
        std::vector<std::function<void()>> m_pending_tasks;

        Timer* m_timer = nullptr;

        ReactorType m_reactor_type = SubReactor;
    };

    class CoroutineTaskQueue {
    public:
        typedef Mutex MutexType;

        CoroutineTaskQueue() = default;

        ~CoroutineTaskQueue();

        void push(FdEvent* fd);

        FdEvent* pop();

    public:
        static CoroutineTaskQueue* GetCoroutineTaskQueue();

    private:
        std::queue<FdEvent*> m_task;          // FdEvent queue
        MutexType m_mutex;                    // mutex
    };

}

#endif //Z_RPC_REACTOR_H
