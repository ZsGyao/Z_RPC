/**
  ******************************************************************************
  * @file           : http_response.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-18
  ******************************************************************************
  */

#ifndef ZRPC_HTTP_RESPONSE_H
#define ZRPC_HTTP_RESPONSE_H

#include <memory>
#include "src/network/abstract_data.h"
#include "src/network/http/http_define.h"

namespace zrpc {
    class HttpResponse : public AbstractData {
    public:
        typedef std::shared_ptr<HttpResponse> ptr;

    public:
        std::string        m_response_version;
        int                m_response_code;
        std::string        m_response_info;
        HttpResponseHeader m_response_header;
        std::string        m_response_body;
    };
}

#endif //ZRPC_HTTP_RESPONSE_H
