/**
  ******************************************************************************
  * @file           : http_dispatcher.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-19
  ******************************************************************************
  */

#ifndef ZRPC_HTTP_DISPATCHER_H
#define ZRPC_HTTP_DISPATCHER_H

#include <map>
#include <string>
#include "src/network/abstract_data.h"
#include "src/network/http/http_servlet.h"
#include "src/network/abstract_dispatcher.h"
#include "src/network/tcp/tcp_connection.h"

namespace zrpc {
    class HttpDispatcher : public AbstractDispatcher {
    public:
        HttpDispatcher() = default;

        ~HttpDispatcher() = default;

        void dispatch(AbstractData* data, TcpConnection* conn);

        void registerServlet(const std::string& path, HttpServlet::ptr servlet);

    public:
        std::map<std::string, HttpServlet::ptr> m_servlets;
    };
}

#endif //ZRPC_HTTP_DISPATCHER_H
