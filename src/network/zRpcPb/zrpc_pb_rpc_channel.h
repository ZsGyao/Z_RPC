/**
  ******************************************************************************
  * @file           : zrpc_pb_rpc_channel.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-20
  ******************************************************************************
  */

#ifndef ZRPC_ZRPC_PB_RPC_CHANNEL_H
#define ZRPC_ZRPC_PB_RPC_CHANNEL_H

#include <memory>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "src/network/net_address.h"

namespace zrpc {
    class ZRpcPbRpcChannel : public google::protobuf::RpcChannel {

    public:
        typedef std::shared_ptr<ZRpcPbRpcChannel> ptr;
        ZRpcPbRpcChannel(NetAddress::ptr addr);
        ~ZRpcPbRpcChannel() = default;

        void CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller,
                        const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        google::protobuf::Closure* done);

    private:
        NetAddress::ptr m_addr;
    };
}

#endif //ZRPC_ZRPC_PB_RPC_CHANNEL_H
