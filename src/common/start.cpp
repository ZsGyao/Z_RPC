/**
  ******************************************************************************
  * @file           : start.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-14
  ******************************************************************************
  */

#include "src/common/start.h"
#include "src/coroutine/hook.h"
#include "src/common/log.h"
#include <iostream>

namespace zrpc {
    extern zrpc::Logger::ptr zRpcLogger;

    void InitConfig(const char* file) {
        zrpc::SetHook(true);
    }

    void StartRpcServer() {

        zRpcLogger->start();
        AppInfoLog << "RpcServer start";
      //  zRpcServer->start();
    }

    //   TcpServer::ptr GetServer();

    int GetIOThreadPoolSize();

    Config::ptr GetConfig() {
        return zRpcConfig;
    }

    void AddTimerEvent(TimerEvent::ptr event) {

    }
}