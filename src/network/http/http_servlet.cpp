/**
  ******************************************************************************
  * @file           : http_servlet.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-19
  ******************************************************************************
  */

#include "src/network/http/http_servlet.h"
#include "src/network/http/http_define.h"
#include "src/common/log.h"

namespace zrpc {
    /* declare in http_define.cpp */
    extern const char* default_html_template;
    extern std::string content_type_text;


    void HttpServlet::handleNotFound(HttpRequest* req, HttpResponse* res) {
        DebugLog << "return 404 html";
        setHttpCode(res, HTTP_NOTFOUND);
        char buf[512];
        sprintf(buf, default_html_template, std::to_string(HTTP_NOTFOUND).c_str(), httpCodeToString(HTTP_NOTFOUND).c_str());
        res->m_response_body = std::string(buf);
        res->m_response_header.m_maps["Content-Type"] = content_type_text;
        res->m_response_header.m_maps["Content-Length"]= std::to_string(res->m_response_body.length());
    }

    void HttpServlet::setHttpCode(HttpResponse* res, HttpCode code) {
        res->m_response_code = code;
        res->m_response_info = httpCodeToString(code);
    }

    void HttpServlet::setHttpContentType(HttpResponse* res, const std::string& content_type) {
        res->m_response_header.m_maps["Content-Type"] = content_type;
    }

    void HttpServlet::setHttpBody(HttpResponse* res, const std::string& body) {
        res->m_response_body = body;
        res->m_response_header.m_maps["Content-Length"]= std::to_string(res->m_response_body.length());
    }

    void HttpServlet::setCommParam(HttpRequest* req, HttpResponse* res) {
        DebugLog << "set response version=" << req->m_request_version;
        res->m_response_version = req->m_request_version;
        res->m_response_header.m_maps["Connection"]= req->m_request_header.m_maps["Connection"];
    }

    void NotFoundHttpServlet::handle(HttpRequest* req, HttpResponse* res) {
        handleNotFound(req, res);
    }

    std::string NotFoundHttpServlet::getServletName() {
        return "NotFoundHttpServlet";
    }
}