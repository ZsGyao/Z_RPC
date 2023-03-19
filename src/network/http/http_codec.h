/**
  ******************************************************************************
  * @file           : http_codec.h
  * @author         : zgys
  * @brief          : HTTP编解码器
  * @attention      : None
  * @date           : 23-3-18
  ******************************************************************************
  */

#ifndef ZRPC_HTTP_CODEC_H
#define ZRPC_HTTP_CODEC_H

#include <string>
#include "src/network/abstract_codec.h"
#include "src/network/tcp/tcp_buffer.h"
#include "src/network/http/http_request.h"

namespace zrpc {

    class HttpCodeC : public AbstractCodeC {
    public:
        HttpCodeC() = default;

        ~HttpCodeC() = default;

        /**
         *
         * @param[in,out] buf
         * @param[in] data
         */
        void encode(TcpBuffer *buf, AbstractData *data) override;

        /**
         *
         * @param[in] buf
         * @param[in,out] data
         */
        void decode(TcpBuffer *buf, AbstractData *data) override;

        ProtocalType getProtocalType() override;

    private:
        /* 解析首行 example: GET https://cn.bing.com/?mkt=zh-CN HTTP/1.1  */
        bool parseHttpRequestLine(HttpRequest *request, const std::string &tmp);

        bool parseHttpRequestHeader(HttpRequest *requset, const std::string &tmp);

        bool parseHttpRequestContent(HttpRequest *requset, const std::string &tmp);
    };
}

#endif //ZRPC_HTTP_CODEC_H
