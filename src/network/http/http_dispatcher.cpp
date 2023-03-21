/**
  ******************************************************************************
  * @file           : http_dispatcher.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#include "src/common/log.h"
#include "src/network/http/http_dispatcher.h"
#include "src/common/msg_req.h"

namespace zrpc {

    void HttpDispatcher::dispatch(AbstractData* data, TcpConnection* conn) {
        HttpRequest* resquest = dynamic_cast<HttpRequest*>(data);
        HttpResponse response;
        Coroutine::GetCurrentCoroutine()->getRunTime()->m_msg_no = MsgReqUtil::genMsgNumber();
        setCurrentRunTime(Coroutine::GetCurrentCoroutine()->getRunTime());

        InfoLog << "begin to dispatch client http request, msgno=" << Coroutine::GetCurrentCoroutine()->getRunTime()->m_msg_no;

        std::string url_path = resquest->m_request_path;
        if (!url_path.empty()) {
            auto it = m_servlets.find(url_path);
            if (it == m_servlets.end()) {
                ErrorLog << "404, url path{ " << url_path << "}, msgno=" << Coroutine::GetCurrentCoroutine()->getRunTime()->m_msg_no;
                NotFoundHttpServlet servlet;
                Coroutine::GetCurrentCoroutine()->getRunTime()->m_interface_name = servlet.getServletName();
                servlet.setCommParam(resquest, &response);
                servlet.handle(resquest, &response);
            } else {

                Coroutine::GetCurrentCoroutine()->getRunTime()->m_interface_name = it->second->getServletName();
                it->second->setCommParam(resquest, &response);
                it->second->handle(resquest, &response);
            }
        }

        conn->getCodec()->encode(conn->getOutBuffer(), &response);

        InfoLog << "end dispatch client http request, msgno=" << Coroutine::GetCurrentCoroutine()->getRunTime()->m_msg_no;

    }

    void HttpDispatcher::registerServlet(const std::string& path, HttpServlet::ptr servlet) {
        auto it = m_servlets.find(path);
        if (it == m_servlets.end()) {
            DebugLog << "register servlet success to path {" << path << "}";
            m_servlets[path] = servlet;
        } else {
            ErrorLog << "failed to register, beacuse path {" << path << "} has already register sertlet";
        }
    }
}