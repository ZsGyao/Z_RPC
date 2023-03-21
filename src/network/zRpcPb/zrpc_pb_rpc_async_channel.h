/**
  ******************************************************************************
  * @file           : zrpc_pb_rpc_async_channel.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#ifndef ZRPC_ZRPC_PB_RPC_ASYNC_CHANNEL_H
#define ZRPC_ZRPC_PB_RPC_ASYNC_CHANNEL_H

#include <google/protobuf/service.h>
#include "src/network/net_address.h"
#include "src/network/tcp/io_thread.h"
#include "src/coroutine/coroutine.h"
#include "src/network/zRpcPb/zrpc_pb_rpc_channel.h"


namespace zrpc {
    class ZRpcPbRpcAsyncChannel : public google::protobuf::RpcChannel , public std::enable_shared_from_this<ZRpcPbRpcAsyncChannel> {

    public:
        typedef std::shared_ptr<ZRpcPbRpcAsyncChannel> ptr;
        typedef std::shared_ptr<google::protobuf::RpcController> con_ptr;
        typedef std::shared_ptr<google::protobuf::Message> msg_ptr;
        typedef std::shared_ptr<google::protobuf::Closure> clo_ptr;

        ZRpcPbRpcAsyncChannel(NetAddress::ptr addr);
        ~ZRpcPbRpcAsyncChannel();

        void CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller,
                        const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        google::protobuf::Closure* done) override;


        ZRpcPbRpcChannel* getRpcChannel();

        // must call saveCallee before CallMethod
        // in order to save shared_ptr count of req res controller
        void saveCallee(con_ptr controller, msg_ptr req, msg_ptr res, clo_ptr closure);

        void wait();

    public:
        void setFinished(bool value);

        bool getNeedResume();

        IOThread* getIOThread();

        Coroutine* getCurrentCoroutine();

        google::protobuf::RpcController* getControllerPtr();

        google::protobuf::Message* getRequestPtr();

        google::protobuf::Message* getResponsePtr();

        google::protobuf::Closure* getClosurePtr();

    private:
        ZRpcPbRpcChannel::ptr m_rpc_channel;
        Coroutine::ptr        m_pending_cor;
        Coroutine*            m_current_cor      = nullptr;
        IOThread*             m_current_iothread = nullptr;
        bool                  m_is_finished      = false;
        bool                  m_need_resume      = false;
        bool                  m_is_pre_set       = false;

    private:
        con_ptr m_controller;
        msg_ptr m_req;
        msg_ptr m_res;
        clo_ptr m_closure;
    };
}

#endif //ZRPC_ZRPC_PB_RPC_ASYNC_CHANNEL_H
