/**
  ******************************************************************************
  * @file           : tcp_server.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#ifndef ZRPC_TCP_SERVER_H
#define ZRPC_TCP_SERVER_H

#include <memory>
#include <google/protobuf/service.h>
#include "src/network/net_address.h"
#include "src/network/abstract_codec.h"
#include "src/network/http/http_servlet.h"
#include "src/network/tcp/tcp_connection.h"
#include "src/coroutine/coroutine.h"
#include "src/network/tcp/io_thread.h"
#include "src/network/tcp/tcp_connect_time_wheel.h"
#include "src/network/abstract_dispatcher.h"

namespace zrpc {
    class TcpAcceptor {
    public:
        typedef std::shared_ptr<TcpAcceptor> ptr;
        explicit TcpAcceptor(NetAddress::ptr net_addr);

        void init();

        int toAccept();

        ~TcpAcceptor();

        NetAddress::ptr getPeerAddr() {
            return m_peer_addr;
        }

        NetAddress::ptr geLocalAddr() {
            return m_local_addr;
        }

    private:
        int m_family = -1;
        int m_fd     = -1;

        NetAddress::ptr m_local_addr = nullptr;
        NetAddress::ptr m_peer_addr  = nullptr;
    };


    class TcpServer {
    public:
        typedef std::shared_ptr<TcpServer> ptr;

        TcpServer(NetAddress::ptr addr, ProtocalType type = zRpcPb_Protocal);

        ~TcpServer();

        void start();

        void addCoroutine(zrpc::Coroutine::ptr cor);

        bool registerService(std::shared_ptr<google::protobuf::Service> service);

        bool registerHttpServlet(const std::string& url_path, HttpServlet::ptr servlet);

        TcpConnection::ptr addClient(IOThread* io_thread, int fd);

        void freshTcpConnection(TcpTimeWheel::TcpConnectionSlot::ptr slot);

    public:
        AbstractDispatcher::ptr getDispatcher();

        AbstractCodeC::ptr getCodeC();

        NetAddress::ptr getPeerAddr();

        NetAddress::ptr getLocalAddr();

        IOThreadPool::ptr getIOThreadPool();

        TcpTimeWheel::ptr getTimeWheel();

    private:
        void MainAcceptCorFunc();

        void ClearClientTimerFunc();

    private:
        NetAddress::ptr  m_addr;
        TcpAcceptor::ptr m_acceptor;
        int m_tcp_counts = 0;
        Reactor* m_main_reactor = nullptr;
        bool m_is_stop_accept = false;
        Coroutine::ptr          m_accept_cor;
        AbstractDispatcher::ptr m_dispatcher;
        AbstractCodeC::ptr      m_codec;
        IOThreadPool::ptr       m_io_pool;

        ProtocalType m_protocal_type = zRpcPb_Protocal;

        TcpTimeWheel::ptr m_time_wheel;
        std::map<int, std::shared_ptr<TcpConnection>> m_clients;
        TimerEvent::ptr m_clear_client_timer_event = nullptr;
    };
}

#endif //ZRPC_TCP_SERVER_H
