/**
  ******************************************************************************
  * @file           : http_request.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-18
  ******************************************************************************
  */

#ifndef ZRPC_HTTP_REQUEST_H
#define ZRPC_HTTP_REQUEST_H

#include <string>
#include <memory>
#include <map>

#include "src/network/abstract_data.h"
#include "src/network/http/http_define.h"

namespace zrpc {

    class HttpRequest : public AbstractData {
    public:
        typedef std::shared_ptr<HttpRequest> ptr;

    public:
        HttpMethod        m_request_method;   // GET or POST
        std::string       m_request_path;
        std::string       m_request_query;
        std::string       m_request_version;
        HttpRequestHeader m_request_header;
        std::string       m_request_body;

        std::map<std::string, std::string> m_query_maps;
    };

}

#endif //ZRPC_HTTP_REQUEST_H
