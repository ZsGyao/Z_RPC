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

namespace zrpc {

    enum IOEvent {
        READ   = EPOLLIN,
        WRITE   = EPOLLOUT,
        ETModel = EPOLLET // 边沿触发

    };
}

#endif //Z_RPC_FD_EVENT_H
