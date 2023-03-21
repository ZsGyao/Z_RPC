/**
  ******************************************************************************
  * @file           : start.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-14
  ******************************************************************************
  */


#ifndef Z_RPC_START_H
#define Z_RPC_START_H

#include "src/common/config.h"
#include "src/network/timer.h"
#include "src/network/tcp/tcp_server.h"

namespace zrpc {
    /**
     * @brief 初始化配置
     * @param file 配置文件路径
     */
//    void InitConfig(const char* file);

    void StartRpcServer();

    TcpServer::ptr GetServer();

    int GetIOThreadPoolSize();

    Config::ptr GetConfig();

    void AddTimerEvent(TimerEvent::ptr event);
}

#endif //Z_RPC_START_H
