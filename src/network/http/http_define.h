/**
  ******************************************************************************
  * @file           : http_define.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-18
  ******************************************************************************
  */

#ifndef ZRPC_HTTP_DEFINE_H
#define ZRPC_HTTP_DEFINE_H

#include <map>
#include <string>

namespace zrpc {

    extern std::string g_CRLF;
    extern std::string g_CRLF_DOUBLE;

    extern std::string content_type_text;
    extern const char* default_html_template;

    /*  HTTP 请求方法
        GET  -> GET 方法的首要目的是 获取资源
        POST -> POST 方法的首要目的是 提交，POST 方法一般用于添加资源 */
    enum HttpMethod {
        GET  = 1,
        POST = 2,
    };

    /* HTTP Status Code, 表示网页服务器超文本传输协议响应状态的3位数字代码*/
    enum HttpCode {
        HTTP_OK                  = 200,
        HTTP_BADREQUEST          = 400,
        HTTP_FORBIDDEN           = 403,
        HTTP_NOTFOUND            = 404,
        HTTP_INTERNALSERVERERROR = 500,
    };

    std::string httpCodeToString(HttpCode code);

    class HttpHeaderComm {
    public:
        HttpHeaderComm() = default;

        virtual ~HttpHeaderComm() = default;

        int getHeaderTotalLength();

        // virtual void storeToMap() = 0;

        std::string getValue(const std::string& key);

        void setKeyValue(const std::string& key, const std::string& value);

        std::string toHttpString();

    public:
        std::map<std::string, std::string> m_maps;
    };

    class HttpRequestHeader : public HttpHeaderComm {
    public:
        // std::string m_accept;
        // std::string m_referer;
        // std::string m_accept_language;
        // std::string m_accept_charset;
        // std::string m_user_agent;
        // std::string m_host;
        // std::string m_content_type;
        // std::string m_content_length;
        // std::string m_connection;
        // std::string m_cookie;

//  public:
//   void storeToMap();
    };

    class HttpResponseHeader : public HttpHeaderComm {
//  public:
//   std::string m_server;
//   std::string m_content_type;
//   std::string m_content_length;
//   std::string m_set_cookie;
//   std::string m_connection;

//  public:
//   void storeToMap();
    };

}

#endif //ZRPC_HTTP_DEFINE_H
