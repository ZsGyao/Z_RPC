/**
  ******************************************************************************
  * @file           : zrpc_pb_rpc_closure.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#ifndef ZRPC_ZRPC_PB_RPC_CLOSURE_H
#define ZRPC_ZRPC_PB_RPC_CLOSURE_H

#include <memory>
#include <functional>

#include <google/protobuf/stubs/callback.h>

namespace zrpc {
    class ZRpcPbRpcClosure : public google::protobuf::Closure {
    public:
        typedef std::shared_ptr<ZRpcPbRpcClosure> ptr;
        explicit ZRpcPbRpcClosure(std::function<void()> cb) : m_cb(cb) {

        }

        ~ZRpcPbRpcClosure() = default;

        void Run() {
            if(m_cb) {
                m_cb();
            }
        }

    private:
        std::function<void()> m_cb = nullptr;
    };
}

#endif //ZRPC_ZRPC_PB_RPC_CLOSURE_H
