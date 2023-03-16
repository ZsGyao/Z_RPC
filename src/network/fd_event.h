/**
  ******************************************************************************
  * @file           : fd_event.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-12
  ******************************************************************************
  */


#ifndef Z_RPC_FD_EVENT_H
#define Z_RPC_FD_EVENT_H

#include <sys/epoll.h>
#include <memory>
#include <functional>
#include "src/network/mutex.hpp"
#include "src/common/singleton.hpp"
#include "src/network/reactor.h"

namespace zrpc {

    enum IOEvent {
        READ    = EPOLLIN,
        WRITE   = EPOLLOUT,
        ETModel = EPOLLET // 边沿触发
    };

    class FdEvent : public std::enable_shared_from_this<FdEvent> {
    public:
        typedef std::shared_ptr<FdEvent> ptr;

        FdEvent(zrpc::Reactor* reactor, int fd = -1);

        FdEvent(int fd);

        virtual ~FdEvent();

        /**
         * @brief 根据传入flag执行相应事件回调
         * @param flag READ   -> m_read_callback()
         *             WRITE  -> m_write_callback();
         *             Other  -> ERROR
         */
        void handleEvent(int flag);

        /**
         * @brief 根据传入的flag事件设置相应事件的回调
         * @param flag READ or WRITE
         * @param cb 读事件回调 or 写事件回调
         */
        void setCallBack(IOEvent flag, std::function<void()> cb);

        /**
         * @brief 返回事件的回调
         * @param flag
         * @return
         */
        std::function<void()> getCallBack(IOEvent flag) const;

        void addListenEvents(IOEvent event);

        void delListenEvents(IOEvent event);

        /**
         * @brief 将fd监听的Event注册到reactor上
         */
        void updateToReactor();

        /**
         * @brief 从reactor中取消注册
         * @attention 同时会将此fd监听的事件清空，读回调与写回调清空
         */
        void unregisterFromReactor ();

        int getFd() const;

        void setFd(const int fd);

        int getListenEvents() const;

        Reactor* getReactor() const;

        void setReactor(Reactor* r);

        /**
         * @brief 将fd设置为非阻塞
         */
        void setNonBlock();

        bool isNonBlock();

        void setCoroutine(Coroutine* cor);

        Coroutine* getCoroutine();

        void clearCoroutine();

    public:
        Mutex m_mutex;

    protected:
        int m_fd = -1;                            // fd
        std::function<void()> m_read_callback;    // 此fd监听的读事件要执行的回调
        std::function<void()> m_write_callback;   // 此fd监听的写事件要执行的回调

        int m_listen_events = 0;                  // 监听的事件 read write

        Reactor*   m_reactor = nullptr;           // reactor
        Coroutine* m_coroutine = nullptr;         // 执行fd的协程
    };


    class FdEventContainer {
    public:
        FdEventContainer(int size);

        /**
         * @brief 获取fd事件
         * @param fd
         * @return
         * @attention 1.5倍扩容
         */
        FdEvent::ptr getFdEvent(int fd);

    public:
        static FdEventContainer* GetFdContainer();

    private:
        RWMutex m_mutex;
        std::vector<FdEvent::ptr> m_fds;
    };

    typedef Singleton<FdEventContainer> Singleton_FdEventContainer;
}



#endif //Z_RPC_FD_EVENT_H
