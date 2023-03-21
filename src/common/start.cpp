/**
  ******************************************************************************
  * @file           : start.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-14
  ******************************************************************************
  */

#include <google/protobuf/service.h>
#include "src/common/start.h"
#include "src/coroutine/hook.h"
#include "src/common/log.h"
#include <iostream>

namespace zrpc {
    extern zrpc::Logger::ptr    zRpcLogger; // declear in config.cpp
    extern zrpc::TcpServer::ptr zRpcServer;

//    void InitConfig(const char* file) {
//        zrpc::SetHook(true);
//    }

    void StartRpcServer() {
        zrpc::SetHook(true);
        zRpcLogger->start();
        zRpcServer->start();
        AppInfoLog << "RpcServer start";
    }

    TcpServer::ptr GetServer() {
        return zRpcServer;
    }

    int GetIOThreadPoolSize() {
        return zRpcServer->getIOThreadPool()->getIOThreadPoolSize();
    }

    Config::ptr GetConfig() {
        return zRpcConfig;
    }

    void AddTimerEvent(TimerEvent::ptr event) {

    }
}