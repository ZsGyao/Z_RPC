/**
  ******************************************************************************
  * @file           : zrpc_pb_rpc_controller.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#include "src/network/zRpcPb/zrpc_pb_rpc_controller.h"

namespace zrpc {
    void ZRpcPbRpcController::Reset() {}

    bool ZRpcPbRpcController::Failed() const {
        return m_is_failed;
    }

    std::string ZRpcPbRpcController::ErrorText() const {
        return m_error_info;
    }

    void ZRpcPbRpcController::StartCancel() {}

    void ZRpcPbRpcController::SetFailed(const std::string& reason) {
        m_is_failed  = true;
        m_error_info = reason;
    }

    bool ZRpcPbRpcController::IsCanceled() const {
        return false;
    }

    void ZRpcPbRpcController::NotifyOnCancel(google::protobuf::Closure* callback) {

    }

    int ZRpcPbRpcController::ErrorCode() const {
        return m_error_code;
    }

    void ZRpcPbRpcController::SetErrorCode(const int error_code) {
        m_error_code = error_code;
    }

    const std::string& ZRpcPbRpcController::MsgSeq() const {
        return m_msg_req;
    }

    void ZRpcPbRpcController::SetMsgReq(const std::string& msg_req) {
        m_msg_req = msg_req;
    }

    void ZRpcPbRpcController::SetError(const int err_code, const std::string& err_info) {
        SetFailed(err_info);
        SetErrorCode(err_code);
    }

    void ZRpcPbRpcController::SetPeerAddr(NetAddress::ptr addr) {
        m_peer_addr = addr;
    }

    void ZRpcPbRpcController::SetLocalAddr(NetAddress::ptr addr) {
        m_local_addr = addr;
    }

    NetAddress::ptr ZRpcPbRpcController::PeerAddr() {
        return m_peer_addr;
    }

    NetAddress::ptr ZRpcPbRpcController::LocalAddr() {
        return m_local_addr;
    }

    void ZRpcPbRpcController::SetTimeout(const int timeout) {
        m_timeout = timeout;
    }

    int ZRpcPbRpcController::Timeout() const {
        return m_timeout;
    }

    void ZRpcPbRpcController::SetMethodName(const std::string& name) {
        m_method_name = name;
    }

    std::string ZRpcPbRpcController::GetMethodName() {
        return m_method_name;
    }

    void ZRpcPbRpcController::SetMethodFullName(const std::string& name) {
        m_full_name = name;
    }

    std::string ZRpcPbRpcController::GetMethodFullName() {
        return m_full_name;
    }
}