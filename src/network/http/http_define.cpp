/**
  ******************************************************************************
  * @file           : http_define.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-18
  ******************************************************************************
  */

#include "src/network/http/http_define.h"
#include <string>
#include <sstream>

namespace zrpc {
    std::string g_CRLF = "\r\n";
    std::string g_CRLF_DOUBLE = "\r\n\r\n";

    std::string content_type_text = "text/html;charset=utf-8";
    const char* default_html_template = "<html><body><h1>%s</h1><p>%s</p></body></html>";

    std::string httpCodeToString(HttpCode code) {
        switch (code)
        {
            case HTTP_OK:
                return "OK";

            case HTTP_BADREQUEST:
                return "Bad Request";

            case HTTP_FORBIDDEN:
                return "Forbidden";

            case HTTP_NOTFOUND:
                return "Not Found";

            case HTTP_INTERNALSERVERERROR:
                return "Internal Server Error";

            default:
                return "UnKnown code";
        }
    }

    int HttpHeaderComm::getHeaderTotalLength() {
        int len = 0;
        for (const auto& it : m_maps) {
            /* head:message\r\n */
            len += it.first.length() + 1 + it.second.length() + 2;
        }
        return len;
    }

    // virtual void storeToMap() = 0;

    std::string HttpHeaderComm::getValue(const std::string& key) {
        return m_maps[key];
    }

    void HttpHeaderComm::setKeyValue(const std::string& key, const std::string& value) {
        m_maps[key] = value;
    }

    std::string HttpHeaderComm::toHttpString() {
        std::stringstream ss;
        for (const auto& it : m_maps) {
            ss << it.first << ":" << it.second << "\r\n";
        }
        return ss.str();
    }

    // void HttpRequestHeader::storeToMap() {
//   m_maps["Accept"] = m_accept;
//   m_maps["Referer"] = m_referer;
//   m_maps["Accept-Language"] = m_accept_language;
//   m_maps["Accept-Charset"] = m_accept_charset;
//   m_maps["User-Agent"] = m_user_agent;
//   m_maps["Host"] = m_host;
//   m_maps["Content-Type"] = m_content_type;
//   m_maps["Content-Length"] = m_content_length;
//   m_maps["Connection"] = m_connection;
//   m_maps["Cookie"] = m_cookie;
// }


// void HttpResponseHeader::storeToMap() {
//   m_maps["Server"] = m_server;
//   m_maps["Content-Type"] = m_content_type;
//   m_maps["Content-Length"] = m_content_length;
//   m_maps["Connection"] = m_connection;
//   m_maps["Set-Cookie"] = m_set_cookie;
// }

}