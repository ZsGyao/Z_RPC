/**
  ******************************************************************************
  * @file           : zrpc_pb_rpc_controller.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#ifndef ZRPC_ZRPC_PB_RPC_CONTROLLER_H
#define ZRPC_ZRPC_PB_RPC_CONTROLLER_H

#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <memory>
#include "src/network/net_address.h"

namespace zrpc {
    class ZRpcPbRpcController : public google::protobuf::RpcController {

    public:
        typedef std::shared_ptr<ZRpcPbRpcController> ptr;
        // Client-side methods ---------------------------------------------

        ZRpcPbRpcController() = default;

        ~ZRpcPbRpcController() = default;

        void Reset() override;

        bool Failed() const override;

        // Server-side methods ---------------------------------------------

        std::string ErrorText() const override;

        void StartCancel() override;

        void SetFailed(const std::string& reason) override;

        bool IsCanceled() const override;

        void NotifyOnCancel(google::protobuf::Closure* callback) override;


        // common methods

        int ErrorCode() const;

        void SetErrorCode(const int error_code);

        const std::string& MsgSeq() const;

        void SetMsgReq(const std::string& msg_req);

        void SetError(const int err_code, const std::string& err_info);

        void SetPeerAddr(NetAddress::ptr addr);

        void SetLocalAddr(NetAddress::ptr addr);

        NetAddress::ptr PeerAddr();

        NetAddress::ptr LocalAddr();

        void SetTimeout(const int timeout);

        int Timeout() const;

        void SetMethodName(const std::string& name);

        std::string GetMethodName();

        void SetMethodFullName(const std::string& name);

        std::string GetMethodFullName();



    private:
        int m_error_code = 0;           // error_code, identify one specific error
        std::string m_error_info;       // error_info, details description of error
        std::string m_msg_req;          // msg_req, identify once rpc request and response
        bool m_is_failed = false;
        bool m_is_canceled = false;
        NetAddress::ptr m_peer_addr;
        NetAddress::ptr m_local_addr;

        int m_timeout = 5000;           // max call rpc timeout
        std::string m_method_name;      // method name
        std::string m_full_name;        // full name, like server.method_name


    };
}

#endif //ZRPC_ZRPC_PB_RPC_CONTROLLER_H
