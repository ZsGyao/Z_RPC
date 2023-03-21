/**
  ******************************************************************************
  * @file           : zrpc_pb_rpc_channel.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-20
  ******************************************************************************
  */

#include "src/network/zRpcPb/zrpc_pb_rpc_channel.h"
#include "src/network/zRpcPb/zrpc_pb_data.h"
#include "src/common/log.h"
#include "src/network/zRpcPb/zrpc_pb_rpc_controller.h"
#include "src/network/tcp/tcp_client.h"
#include "src/network/tcp/tcp_server.h"
#include "src/common/run_time.h"
#include "src/common/msg_req.h"
#include "src/coroutine/coroutine.h"
#include "src/common/error_code.h"

namespace zrpc {
    ZRpcPbRpcChannel::ZRpcPbRpcChannel(NetAddress::ptr addr) : m_addr(std::move(addr)) {}

    void  ZRpcPbRpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done) {
        ZRpcPbStruct pb_struct;
        auto rpc_controller = dynamic_cast<ZRpcPbRpcController*>(controller);
        if (!rpc_controller) {
            ErrorLog << "call failed. failed to dynamic cast TinyPbRpcController";
            return;
        }

        TcpClient::ptr m_client = std::make_shared<TcpClient>(m_addr);
        rpc_controller->SetLocalAddr(m_client->getLocalAddr());
        rpc_controller->SetPeerAddr(m_client->getPeerAddr());

        pb_struct.service_full_name = method->full_name();
        DebugLog << "call service_name = " << pb_struct.service_full_name;
        if (!request->SerializeToString(&(pb_struct.pb_data))) {
            ErrorLog << "serialize send package error";
            return;
        }

        if (!rpc_controller->MsgSeq().empty()) {
            pb_struct.msg_req = rpc_controller->MsgSeq();
        } else {
            // get current coroutine's msgno to set this request
            auto run_time = reinterpret_cast<RunTime*>(Coroutine::GetCurrentRunTime);
            if(run_time != nullptr && !run_time->m_msg_no.empty()) {
                pb_struct.msg_req = run_time->m_msg_no;
                DebugLog << "get from RunTime succ, msgno = " << pb_struct.msg_req;
            } else {
                pb_struct.msg_req = MsgReqUtil::genMsgNumber();
                DebugLog << "get from RunTime error, generate new msgno = " << pb_struct.msg_req;
            }
            rpc_controller->SetMsgReq(pb_struct.msg_req);
        }

        AbstractCodeC::ptr m_codec = m_client->getConnection()->getCodec();
        m_codec->encode(m_client->getConnection()->getOutBuffer(), &pb_struct);
        if (!pb_struct.encode_success) {
            rpc_controller->SetError(ERROR_FAILED_ENCODE, "encode zRpcPb data error");
            return;
        }

        InfoLog << "============================================================";
        InfoLog << pb_struct.msg_req << "|" << rpc_controller->PeerAddr()->toString()
                << "|. Set client send request data:" << request->ShortDebugString();
        InfoLog << "============================================================";
        m_client->setTimeout(rpc_controller->Timeout());

        ZRpcPbStruct::pb_ptr res_data;
        int rt = m_client->sendAndRecvZRpcPb(pb_struct.msg_req, res_data);
        if (rt != 0) {
            rpc_controller->SetError(rt, m_client->getErrInfo());
            ErrorLog << pb_struct.msg_req << "|call rpc occur client error, service_full_name=" << pb_struct.service_full_name << ", error_code="
                     << rt << ", error_info = " << m_client->getErrInfo();
            return;
        }

        if (!response->ParseFromString(res_data->pb_data)) {
            rpc_controller->SetError(ERROR_FAILED_DESERIALIZE, "failed to deserialize data from server");
            ErrorLog << pb_struct.msg_req << "|failed to deserialize data";
            return;
        }
        if (res_data->err_code != 0) {
            ErrorLog << pb_struct.msg_req << "|server reply error_code=" << res_data->err_code << ", err_info=" << res_data->err_info;
            rpc_controller->SetError(res_data->err_code, res_data->err_info);
            return;
        }

        InfoLog<< "============================================================";
        InfoLog<< pb_struct.msg_req << "|" << rpc_controller->PeerAddr()->toString()
               << "|call rpc server [" << pb_struct.service_full_name << "] succ"
               << ". Get server reply response data:" << response->ShortDebugString();
        InfoLog<< "============================================================";

        // execute callback function
        if (done) {
            done->Run();
        }
    }

}