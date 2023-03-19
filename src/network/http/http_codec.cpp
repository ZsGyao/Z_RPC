/**
  ******************************************************************************
  * @file           : http_codec.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-18
  ******************************************************************************
  */

#include "src/network/http/http_codec.h"
#include "src/common/log.h"
#include "src/network/http/http_request.h"
#include "src/network/http/http_response.h"
#include "src/common/string_uitl.h"
#include <algorithm>

namespace zrpc {
    void  HttpCodeC::encode(TcpBuffer* buf, AbstractData* data) {
        DebugLog << "test encode";
        auto response = dynamic_cast<HttpResponse*>(data);
        response->encode_success = false;

        std::stringstream ss;
        ss << response->m_response_version << " " << response->m_response_code << " "
           << response->m_response_info << "\r\n" << response->m_response_header.toHttpString()
           << "\r\n" << response->m_response_body;
        std::string http_res = ss.str();
        DebugLog << "encode http response is:  " << http_res;

        buf->writeToBuffer(http_res.c_str(), http_res.length());
        DebugLog << "success encode and write to buffer, write index=" << buf->writeIndex();
        response->encode_success = true;
        DebugLog << "test encode end";
    }

    void  HttpCodeC::decode(TcpBuffer* buf, AbstractData* data) {
        DebugLog << "test http decode start";
        std::string strs;
        if (!buf || !data) {
            ErrorLog << "decode error! (TcpBuffer*)buf or (AbstractData*)data is nullptr";
            return;
        }
        auto request = dynamic_cast<HttpRequest*>(data);
        if (!request) {
            ErrorLog << "not HttpRequest type";
            return;
        }

        strs = buf->getBufferString();

        bool is_parse_request_line    = false;
        bool is_parse_request_header  = false;
        bool is_parse_request_content = false;
        // bool is_parse_succ = false;
        int read_size = 0;
        std::string tmp(strs);
        DebugLog << "pending to parse str:" << tmp << ", total size =" << tmp.size();
        int len = tmp.length();
        while (true) {
            if (!is_parse_request_line) {
                /* use find to find g_CRLF, if not contain, return npos*/
                size_t i = tmp.find(g_CRLF);
                if (i == tmp.npos) {
                    DebugLog << "not found CRLF in buffer";
                    return;
                }
                if (i == tmp.length() - 2) {
                    DebugLog << "need to read more data";
                    break;
                }
                is_parse_request_line = parseHttpRequestLine(request, tmp.substr(0, i));
                if (!is_parse_request_line) {
                    return;
                }
                tmp = tmp.substr(i + 2, len - 2 - i);
                len = tmp.length();
                read_size = read_size + i + 2;
            }

            if (!is_parse_request_header) {
                size_t j  = tmp.find(g_CRLF_DOUBLE);
                if (j == tmp.npos) {
                    DebugLog << "not found CRLF CRLF in buffer";
                    return;
                }
                // if (j == tmp.length() - 4) {
                //   DebugLog << "need to read more data";
                //   goto parse_error;
                // }
                is_parse_request_header = parseHttpRequestHeader(request, tmp.substr(0, j));
                if (!is_parse_request_header) {
                    return;
                }
                tmp = tmp.substr(j + 4, len - 4 - j);
                len = tmp.length();
                read_size = read_size + j + 4;
            }
            if (!is_parse_request_content) {
                int content_len = std::atoi(request->m_request_header.m_maps["Content-Length"].c_str());
                if ((int)strs.length() - read_size < content_len) {
                    DebugLog << "need to read more data";
                    return;
                }
                if (request->m_request_method == POST && content_len != 0) {
                    is_parse_request_content = parseHttpRequestContent(request, tmp.substr(0, content_len));
                    if (!is_parse_request_content) {
                        return;
                    }
                    read_size = read_size + content_len;
                } else {
                    is_parse_request_content = true;
                }

            }
            if (is_parse_request_line && is_parse_request_header && is_parse_request_header) {
                DebugLog << "parse http request success, read size is " << read_size << " bytes";
                buf->recycleRead(read_size);
                break;
            }
        }

        request->decode_success = true;
        data = request;

        DebugLog << "test http decode end";
    }

    ProtocalType  HttpCodeC::getProtocalType() {
        return Http_Protocal;
    }

    bool  HttpCodeC::parseHttpRequestLine(HttpRequest* request, const std::string& tmp) {
        size_t s1 = tmp.find_first_of(" ");
        size_t s2 = tmp.find_last_of(" ");

        if (s1 == tmp.npos || s2 == tmp.npos || s1 == s2) {
            ErrorLog << "error read Http Request Line, space is not 2";
            return false;
        }
        std::string method = tmp.substr(0, s1);
        std::transform(method.begin(), method.end(), method.begin(), ::toupper);
        if (method == "GET") {
            request->m_request_method = HttpMethod::GET;
        } else if (method == "POST") {
            request->m_request_method = HttpMethod::POST;
        } else {
            ErrorLog << "parse http request request line error, not support http method:" << method;
            return false;
        }

        std::string version = tmp.substr(s2 + 1, tmp.length() - s2 - 1);
        std::transform(version.begin(), version.end(), version.begin(), toupper);
        if (version != "HTTP/1.1" && version != "HTTP/1.0") {
            ErrorLog << "parse http request, request line error, not support http version:" << version;
            return false;
        }
        request->m_request_version = version;


        std::string url = tmp.substr(s1 + 1, s2 - s1 - 1);
        size_t j = url.find("://");

        if (j != url.npos && j + 3 >= url.length()) {
            ErrorLog << "parse http request request line error, bad url:" << url;
            return false;
        }
        int l = 0;
        if (j == url.npos) {
            DebugLog << "url only have path, url is" << url;
        } else {
            url = url.substr(j + 3, s2 - s1  - j - 4);
            DebugLog << "delete http prefix, url = " << url;
            j = url.find_first_of("/");
            l = url.length();
            if (j == url.npos || j == url.length() - 1) {
                DebugLog << "http request root path, and query is empty";
                return true;
            }
            url = url.substr(j + 1, l - j - 1);
        }

        l = url.length();
        j = url.find_first_of("?");
        if (j == url.npos) {
            request->m_request_path = url;
            DebugLog << "http request path:" << request->m_request_path << "and query is empty";
            return true;
        }
        request->m_request_path = url.substr(0, j);
        request->m_request_query = url.substr(j + 1, l - j - 1);
        DebugLog << "http request path:" << request->m_request_path << ", and query:" << request->m_request_query;
        StringUtil::SplitStrToMap(request->m_request_query, "&", "=", request->m_query_maps);
        return true;

    }

    bool  HttpCodeC::parseHttpRequestHeader(HttpRequest *request, const std::string& str) {
        if (str.empty() || str.length() < 4 || str == "\r\n\r\n") {
            return true;
        }
        std::string tmp = str;
        StringUtil::SplitStrToMap(tmp, "\r\n", ":", request->m_request_header.m_maps);
        return true;
    }

    bool  HttpCodeC::parseHttpRequestContent(HttpRequest* request, const std::string& str) {
        if (str.empty()) {
            return true;
        }
        request->m_request_body = str;
        return true;
    }
}