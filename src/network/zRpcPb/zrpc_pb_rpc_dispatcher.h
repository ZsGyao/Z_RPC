/**
  ******************************************************************************
  * @file           : zrpc_pb_rpc_dispacther.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#ifndef ZRPC_ZRPC_PB_RPC_DISPATCHER_H
#define ZRPC_ZRPC_PB_RPC_DISPATCHER_H

#include <memory>
#include <google/protobuf/service.h>
#include "src/network/abstract_data.h"
#include "src/network/abstract_dispatcher.h"
#include "src/network/tcp/tcp_connection.h"

namespace zrpc {

    class ZRpcPbRpcDispatcher : public AbstractDispatcher {
    public:
        typedef std::shared_ptr<google::protobuf::Service> service_ptr;

        ZRpcPbRpcDispatcher() = default;
        ~ZRpcPbRpcDispatcher() = default;

        void dispatch(AbstractData* data, TcpConnection* conn);

        bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);

        void registerService(service_ptr service);

    public:

        // all services should be registered on there before progress start
        // key: service_name
        std::map<std::string, service_ptr> m_service_map;

    };

}

#endif //ZRPC_ZRPC_PB_RPC_DISPATCHER_H
