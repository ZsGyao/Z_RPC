/**
  ******************************************************************************
  * @file           : zrpc_pb_rpc_async_channel.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "src/common/log.h"
#include "src/common/start.h"
#include "src/network/zRpcPb/zrpc_pb_rpc_async_channel.h"
#include "src/coroutine/coroutine_pool.h"
#include "src/network/zRpcPb/zrpc_pb_rpc_dispatcher.h"
#include "src/network/zRpcPb/zrpc_pb_rpc_controller.h"
#include "src/common/error_code.h"
#include "src/common/msg_req.h"

namespace zrpc {
    ZRpcPbRpcAsyncChannel::ZRpcPbRpcAsyncChannel(NetAddress::ptr addr) {
        m_rpc_channel = std::make_shared<ZRpcPbRpcChannel>(addr);
        m_current_iothread = IOThread::GetCurrentIOThread();
        m_current_cor = Coroutine::GetCurrentCoroutine().get();
    }

    ZRpcPbRpcAsyncChannel::~ZRpcPbRpcAsyncChannel() {
        // DebugLog << "~TinyPbRpcAsyncChannel(), return coroutine";
        zRpcCoroutinePool->returnCoroutine(m_pending_cor);
    }

    ZRpcPbRpcChannel* ZRpcPbRpcAsyncChannel::getRpcChannel() {
        return m_rpc_channel.get();
    }

    void ZRpcPbRpcAsyncChannel::saveCallee(con_ptr controller, msg_ptr req, msg_ptr res, clo_ptr closure) {
        m_controller = controller;
        m_req        = req;
        m_res        = res;
        m_closure    = closure;
        m_is_pre_set = true;
    }

    void ZRpcPbRpcAsyncChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                           google::protobuf::RpcController* controller,
                                           const google::protobuf::Message* request,
                                           google::protobuf::Message* response,
                                           google::protobuf::Closure* done) {

        auto rpc_controller = dynamic_cast<ZRpcPbRpcController*>(controller);
        if (!m_is_pre_set) {
            ErrorLog << "Error! must call [saveCallee()] function before [CallMethod()]";
            rpc_controller = dynamic_cast<ZRpcPbRpcController*>(controller);
            rpc_controller->SetError(ERROR_NOT_SET_ASYNC_PRE_CALL, "Error! must call [saveCallee()] function before [CallMethod()];");
            m_is_finished = true;
            return;
        }
        auto run_time = reinterpret_cast<RunTime *>(Coroutine::GetCurrentRunTime);
        if (run_time) {
            rpc_controller->SetMsgReq(run_time->m_msg_no);
            DebugLog << "get from RunTime success, msgno=" << run_time->m_msg_no;
        } else {
            rpc_controller->SetMsgReq(MsgReqUtil::genMsgNumber());
            DebugLog << "get from RunTime error, generate new msgno=" << rpc_controller->MsgSeq();
        }

        std::shared_ptr<ZRpcPbRpcAsyncChannel> s_ptr = shared_from_this();

        auto cb = [s_ptr, method]() mutable {
            DebugLog << "now execute rpc call method by this thread";
            s_ptr->getRpcChannel()->CallMethod(method, s_ptr->getControllerPtr(), s_ptr->getRequestPtr(), s_ptr->getResponsePtr(), NULL);

            DebugLog << "execute rpc call method by this thread finish";

            auto call_back = [s_ptr]() mutable {
                DebugLog << "async execute rpc call method back old thread";
                // callback function execute in origin thread
                if (s_ptr->getClosurePtr() != nullptr) {
                    s_ptr->getClosurePtr()->Run();
                }
                s_ptr->setFinished(true);

                if (s_ptr->getNeedResume()) {
                    DebugLog << "async execute rpc call method back old thread, need resume";
                    s_ptr->getCurrentCoroutine()->resume();
                }
                s_ptr.reset();
            };

            s_ptr->getIOThread()->getReactor()->addTask(call_back, true);
            s_ptr.reset();
        };
        m_pending_cor = GetServer()->getIOThreadPool()->addCoroutineToRandomThread(cb, false);
    }

    void ZRpcPbRpcAsyncChannel::wait() {
        m_need_resume = true;
        if (m_is_finished) {
            return;
        }
        Coroutine::GetCurrentCoroutine()->yield();
    }

    void ZRpcPbRpcAsyncChannel::setFinished(bool value) {
        m_is_finished = true;
    }

    IOThread* ZRpcPbRpcAsyncChannel::getIOThread() {
        return m_current_iothread;
    }

    Coroutine* ZRpcPbRpcAsyncChannel::getCurrentCoroutine() {
        return m_current_cor;
    }

    bool ZRpcPbRpcAsyncChannel::getNeedResume() {
        return m_need_resume;
    }

    google::protobuf::RpcController* ZRpcPbRpcAsyncChannel::getControllerPtr() {
        return m_controller.get();
    }

    google::protobuf::Message* ZRpcPbRpcAsyncChannel::getRequestPtr() {
        return m_req.get();
    }

    google::protobuf::Message* ZRpcPbRpcAsyncChannel::getResponsePtr() {
        return m_res.get();
    }

    google::protobuf::Closure* ZRpcPbRpcAsyncChannel::getClosurePtr() {
        return m_closure.get();
    }

}