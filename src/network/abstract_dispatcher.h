/**
  ******************************************************************************
  * @file           : abstract_dispatcher.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-19
  ******************************************************************************
  */

#ifndef ZRPC_ABSTRACT_DISPATCHER_H
#define ZRPC_ABSTRACT_DISPATCHER_H

#include <memory>
#include "src/network/abstract_data.h"

namespace zrpc {
    class AbstractDispatcher {
    public:
        typedef std::shared_ptr<AbstractDispatcher> ptr;

        AbstractDispatcher() {}

        virtual ~AbstractDispatcher() {}

        virtual void dispatch(AbstractData* data, TcpConnection* conn) = 0;

    };
}

#endif //ZRPC_ABSTRACT_DISPATCHER_H
