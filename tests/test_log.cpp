/**
  ******************************************************************************
  * @file           : test_log.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-10
  ******************************************************************************
  */

#include "src/common/log.h"
#include "src/common/start.h"
#include "src/network/reactor.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << " ++++++ " << std::endl;

    zrpc::StartRpcServer();

    std::cout << " Log begin " << std::endl;

    DebugLog << "Hello rpc log";
    // AppDebugLog << "Hello app log";

    std::cout << " Log end " << std::endl;
    std::cout << " ------ " << std::endl;
    return 0;
}