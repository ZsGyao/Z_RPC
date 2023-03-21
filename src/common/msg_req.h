/**
  ******************************************************************************
  * @file           : msg_req.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-20
  ******************************************************************************
  */

#ifndef ZRPC_MSG_REQ_H
#define ZRPC_MSG_REQ_H

#include <string>
#include <memory>

namespace zrpc {
    class MsgReqUtil {
    public:
        static std::string genMsgNumber();
    };

}

#endif //ZRPC_MSG_REQ_H
