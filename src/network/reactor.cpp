/**
  ******************************************************************************
  * @file           : reactor.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-13
  ******************************************************************************
  */

#include "src/network/reactor.h"
#include "src/network/timer.h"
#include "src/coroutine/hook.h"
#include "src/common/macro.h"
#include "src/common/log.h"
#include <algorithm>
#include <iostream>
#include <sys/eventfd.h>


extern read_fun_ptr_t  g_sys_read_fun;  // system read function
extern write_fun_ptr_t g_sys_write_fun; // system write function

namespace zrpc {

    /* every thread only have one reactor */
    static thread_local Reactor* t_reactor_ptr = nullptr;
    static thread_local int t_max_epoll_timeout = 10000;   // ms
    static CoroutineTaskQueue* t_coroutine_task_queue = nullptr;

    /* 每个线程创建一个reactor， reactor通常是死循环 */
    Reactor::Reactor() {
        if(t_reactor_ptr != nullptr) {
            ZRPC_ASSERT2(false, "this thread already have a reactor");
        }

        m_tid = gettid();
        DebugLog << "thread [id:" << m_tid << "] create a reactor";
        t_reactor_ptr = this;

        if((m_epfd = epoll_create(1)) <= 0) {
            ZRPC_ASSERT2(false, "epoll_create");
        }
        DebugLog << "The reactor [" << t_reactor_ptr << "] in thread ["
                 << m_tid << "] epoll create [epoll_fd:" << m_epfd << "]";

//        std::cout << "**** Debug [reactor.cpp " << __LINE__ << "], Reactor construct [threadId:"
//                  << gettid() << "] [epoll_fd:" << m_epfd << "] ****" << std::endl;

        /* 用来创建一个用于事件通知的 eventfd 对象, 返回值为一个int型fd，用来引用打开的对象 */
        if((m_wake_fd = eventfd(0, EFD_NONBLOCK)) <= 0) {
            ErrorLog << "start server error. event_fd error, sys error=" << strerror(errno);
            Exit(0);
        }
        DebugLog << "Reactor::Reactor(): wake_fd = " << m_wake_fd;

        addWakeupFd();
    }

    Reactor::~Reactor() {
        std::cout << "Debug [reactor.cpp " << __LINE__ << "], ~Reactor() destroy [epollfd:"
                  << m_epfd << "]" << std::endl;

        DebugLog << "~Reactor epoll_fd = "<< m_epfd;
        close(m_epfd);
        if(m_timer){
            delete m_timer;
            m_timer = nullptr;
        }
        t_reactor_ptr = nullptr;
    }

    void Reactor::addEvent(int fd, epoll_event event, bool is_wakeup) {
        DebugLog << "addEvent to reactor [fd:" << fd;
        if(fd == -1) {
            ErrorLog << "addEvent error, fd invalid, fd = -1";
            return;
        }
        if(isLoopThread()){
            addEventInLoopThread(fd, event);
            return;
        }

        MutexType::Lock lock(m_mutex);
        m_pending_add_fds.insert(std::pair<int, epoll_event>(1, event));
        lock.unlock();

        if(is_wakeup) {
            wakeup();
        }
    }

    void Reactor::delEvent(int fd, bool is_wakeup) {
        if(fd == -1) {
            ErrorLog << "delEvent error, fd invalid, fd = -1";
            return;
        }
        if(isLoopThread()){
            delEventInLoopThread(fd);
            return;
        }

        MutexType::Lock lock(m_mutex);
        m_pending_del_fds.emplace_back(fd);
        lock.unlock();

        if(is_wakeup) {
            wakeup();
        }
    }

    void Reactor::addTask(const std::function<void()>& task, bool is_wakeup) {
        {
            MutexType::Lock lock(m_mutex);
            m_pending_tasks.push_back(task);
        }
        if (is_wakeup) {
            wakeup();
        }
    }

    void Reactor::addTask(std::vector<std::function<void()>> task, bool is_wakeup) {
        if (task.empty()) {
            return;
        }

        {
            MutexType::Lock lock(m_mutex);
            m_pending_tasks.insert(m_pending_tasks.end(), task.begin(), task.end());
        }

        if (is_wakeup) {
            wakeup();
        }
    }

    void Reactor::addCoroutine(zrpc::Coroutine::ptr cor, bool is_wakeup) {
        auto func = [cor](){
            cor->resume();
        };
        addTask(func, is_wakeup);
    }

    void Reactor::wakeup() const {
        if (!m_is_looping) {
            return;
        }

        uint64_t buff = 1;
        if(g_sys_write_fun(m_wake_fd, &buff, 8) != 8) {
            ErrorLog << "write wake_up_fd[" << m_wake_fd <<"] error";
        }
    }

    void Reactor::loop() {
        ZRPC_ASSERT(isLoopThread());
        if(m_is_looping) {
            WarnLog << "This reactor is looping";
            return;
        }
        m_is_looping = true;
        m_stop_flag  = false;

        Coroutine* first_coroutine = nullptr;
        while (!m_stop_flag) {
            const int MAX_EVENT = 10;
            epoll_event re_events[MAX_EVENT + 1];
            if (first_coroutine) {
                first_coroutine->resume();
                first_coroutine = nullptr;
            }

            /* SubReactor only register client_fd, listen read or write event and coroutine tasks */
            if (m_reactor_type != MainReactor) {
                FdEvent* ptr = nullptr;
                while (true) {
                    ptr = CoroutineTaskQueue::GetCoroutineTaskQueue()->pop();
                    if (ptr) {
                        ptr->setReactor(this);
                        ptr->getCoroutine()->resume();
                    } else {
                        break;
                    }
                }
            }
            // 取出任务 execute task in m_pending_tasks
            MutexType::Lock lock(m_mutex);
            std::vector<std::function<void()>> tmp_tasks;
            tmp_tasks.swap(m_pending_tasks);
            lock.unlock();

            for (const auto& task: tmp_tasks) {
                if (task) {
                    task();
                }
            }

            int rt = epoll_wait(m_epfd, re_events, MAX_EVENT, t_max_epoll_timeout);
            if (rt < 0) {
                ErrorLog << "epoll_wait error, errno=" << strerror(errno);
            } else {
                for (int i = 0; i < rt; i++) {
                    epoll_event event = re_events[i];
                    // 事件被唤醒，并且是读事件
                    if (event.data.fd == m_wake_fd && (event.events & READ)) {
                        char buf[8];
                        while (true) {
                            if ((g_sys_read_fun(m_wake_fd, buf, 8) == -1) && errno == EAGAIN) {
                                break;
                            }
                        }
                    } else { // 是监听事件
                        auto ptr = (zrpc::FdEvent*) event.data.ptr;
                        if (ptr != nullptr) {
                            int fd = ptr->getFd();
                            if (!(event.events & EPOLLIN) && (!((event.events & EPOLLOUT)))) {
                                ErrorLog << "socket [" << fd << "] occur other unknow event:[" << event.events
                                         << "], need unregister this socket";
                                delEventInLoopThread(fd);
                            } else {
                                // if register coroutine, pending coroutine to common coroutine_tasks
                                if (ptr->getCoroutine()) {
                                    // the first one coroutine when epoll_wait back, just directly resume by this thread, not add to global CoroutineTaskQueue
                                    // because every operate CoroutineTaskQueue should add mutex lock
                                    if (!first_coroutine) {
                                        first_coroutine = ptr->getCoroutine();
                                        continue;
                                    }
                                    if (m_reactor_type == SubReactor) {
                                        delEventInLoopThread(fd);
                                        ptr->setReactor(nullptr);
                                        CoroutineTaskQueue::GetCoroutineTaskQueue()->push(ptr);
                                    } else {
                                        // main reactor, just resume this coroutine. it is accept coroutine. and Main Reactor only have this coroutine
                                        ptr->getCoroutine()->resume();
                                        if (first_coroutine) {
                                            first_coroutine = nullptr;
                                        }
                                    }
                                } else {
                                    std::function<void()> read_cb;
                                    std::function<void()> write_cb;
                                    read_cb = ptr->getCallBack(READ);
                                    write_cb = ptr->getCallBack(WRITE);
                                    // if timer event, direct execute
                                    if (fd == m_timer_fd) {
                                        read_cb();
                                        continue;
                                    }
                                    if (event.events & EPOLLIN) {
                                         DebugLog << "socket [" << fd << "] occur read event";
                                        MutexType::Lock lock1(m_mutex);
                                        m_pending_tasks.push_back(read_cb);
                                    }
                                    if (event.events & EPOLLOUT) {
                                         DebugLog << "socket [" << fd << "] occur write event";
                                        MutexType::Lock lock1(m_mutex);
                                        m_pending_tasks.push_back(write_cb);
                                    }
                                }
                            }
                        }
                    }
                }
                std::map<int, epoll_event> tmp_add;
                std::vector<int> tmp_del;

                {
                    Mutex::Lock lock1(m_mutex);
                    tmp_add.swap(m_pending_add_fds);
                    m_pending_add_fds.clear();

                    tmp_del.swap(m_pending_del_fds);
                    m_pending_del_fds.clear();
                }
                for (const auto &add_fd: tmp_add) {
                    addEventInLoopThread(add_fd.first, add_fd.second);
                }
                for (const auto &del_fd: tmp_del) {
                    delEventInLoopThread(del_fd);
                }
            }
        }
        DebugLog << "reactor loop end";
        m_is_looping = false;
    }


    void Reactor::stop() {
        if (!m_stop_flag && m_is_looping) {
            m_stop_flag = true;
            wakeup();
        }
    }

    Timer* Reactor::getTimer() {
        if (!m_timer) {
            m_timer = new Timer(this);
            m_timer_fd = m_timer->getFd();
        }
        return m_timer;
    }

    pid_t Reactor::getTid() const {
        return m_tid;
    }

    void Reactor::setReactorType(ReactorType type) {
        m_reactor_type = type;
    }

    Reactor* Reactor::GetReactor() {
        if(t_reactor_ptr == nullptr) {
            WarnLog << "Reactor not exist, create new Reactor";
            t_reactor_ptr = new Reactor();
        }
        return t_reactor_ptr;
    }

    void Reactor::addWakeupFd() {
        DebugLog << " Reactor::addWakeupFd()";

        int op = EPOLL_CTL_ADD;
        epoll_event event;
        event.data.fd = m_wake_fd;
        event.events = EPOLLIN;
        if ((epoll_ctl(m_epfd, op, m_wake_fd, &event)) != 0) {
            ErrorLog << "epoll_ctl error, [fd:" << m_wake_fd << "], errno=" << errno << ", err=" << strerror(errno) ;
        }
        m_fds.push_back(m_wake_fd);
    }

    bool Reactor::isLoopThread() const {
        if(m_tid == gettid()){
            return true;
        }
        return false;
    }

    void Reactor::addEventInLoopThread(int fd, epoll_event event) {
        ZRPC_ASSERT(isLoopThread());
        DebugLog << "Reactor::addEventInLoopThread [fd:" << fd;
        int op = EPOLL_CTL_ADD;
        bool is_add = false;
        auto it = std::find(m_fds.begin(), m_fds.end(), fd); // 是否添加过这个fd
        if (it != m_fds.end()) {                                                   // 添加过
            is_add = true;
            op = EPOLL_CTL_MOD;
        }

        if (epoll_ctl(m_epfd, op, fd, &event) != 0) {
            ErrorLog << "epoll_ctl error, [fd:" << fd << "], sys errinfo = " << strerror(errno);
            return;
        }
        if (!is_add) {
            m_fds.push_back(fd);
        }
        DebugLog << "Reactor::addEventInLoopThread   epoll_ctl add success, [fd:" << fd << "]";
    }

    void Reactor::delEventInLoopThread(int fd) {
        ZRPC_ASSERT(isLoopThread());

        auto it = std::find(m_fds.begin(), m_fds.end(), fd); // 是否添加过这个fd
        if (it == m_fds.end()) {                                                   // 添加过
            DebugLog << "[fd:" << fd << "] not in this loop";
            return;
        }
        int op = EPOLL_CTL_DEL;
        if ((epoll_ctl(m_epfd, op, fd, nullptr)) != 0) {
            ErrorLog << "epoll_ctl error, [fd:" << fd << "], sys errinfo = " << strerror(errno);
        }

        m_fds.erase(it);
        DebugLog << "del success, [fd:" << fd << "]";
    }

    void CoroutineTaskQueue::push(FdEvent* fd) {
        Mutex::Lock lock(m_mutex);
        m_task.push(fd);
        lock.unlock();
    }

    FdEvent* CoroutineTaskQueue::pop() {
        FdEvent* re = nullptr;
        Mutex::Lock lock(m_mutex);
        if (!m_task.empty()) {
            re = m_task.front();
            m_task.pop();
        }
        lock.unlock();

        return re;
    }

    CoroutineTaskQueue* CoroutineTaskQueue::GetCoroutineTaskQueue() {
        if (t_coroutine_task_queue) {
            return t_coroutine_task_queue;
        }
        t_coroutine_task_queue = new CoroutineTaskQueue();
        return t_coroutine_task_queue;
    }

    CoroutineTaskQueue::~CoroutineTaskQueue() {
        delete t_coroutine_task_queue;
        t_coroutine_task_queue = nullptr;
        DebugLog << "~CoroutineTaskQueue() destroy";
    }

}