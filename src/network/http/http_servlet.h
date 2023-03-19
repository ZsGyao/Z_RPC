/**
  ******************************************************************************
  * @file           : http_servlet.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-19
  ******************************************************************************
  */

#ifndef ZRPC_HTTP_SERVLET_H
#define ZRPC_HTTP_SERVLET_H

#include <memory>
#include "src/network/http/http_request.h"
#include "src/network/http/http_response.h"

namespace zrpc {

    class HttpServlet : public std::enable_shared_from_this<HttpServlet> {
    public:
        typedef std::shared_ptr<HttpServlet> ptr;

        HttpServlet() = default;

        virtual ~HttpServlet() = default;

        virtual void handle(HttpRequest* req, HttpResponse* res) = 0;

        virtual std::string getServletName() = 0;

    public:
        static void handleNotFound(HttpRequest* req, HttpResponse* res);

        static void setHttpCode(HttpResponse* res, HttpCode code);

        static void setHttpContentType(HttpResponse* res, const std::string& content_type);

        static void setHttpBody(HttpResponse* res, const std::string& body);

        static void setCommParam(HttpRequest* req, HttpResponse* res);
    };


    class NotFoundHttpServlet: public HttpServlet {
    public:
        NotFoundHttpServlet() = default;

        ~NotFoundHttpServlet() = default;

        void handle(HttpRequest* req, HttpResponse* res) override;

        std::string getServletName() override;

    };
}

#endif //ZRPC_HTTP_SERVLET_H
