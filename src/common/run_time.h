/**
  ******************************************************************************
  * @file           : run_time.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-10
  ******************************************************************************
  */


#ifndef Z_RPC_RUN_TIME_H
#define Z_RPC_RUN_TIME_H

#include <string>

namespace zrpc {
    class RunTime {
    public:
        std::string m_msg_no;
        std::string m_interface_name; // 远程调用的接口名
    };
}

#endif //Z_RPC_RUN_TIME_H
