/**
  ******************************************************************************
  * @file           : tcp_client.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#ifndef ZRPC_TCP_CLIENT_H
#define ZRPC_TCP_CLIENT_H

#include "src/network/net_address.h"
#include "src/network/abstract_data.h"
#include "src/network/abstract_codec.h"
#include "src/network/zRpcPb/zrpc_pb_data.h"
#include "src/network/tcp/tcp_connection.h"

namespace zrpc {
//
// You should use TcpClient in a coroutine(not main coroutine)
//
    class TcpClient {
    public:
        typedef std::shared_ptr<TcpClient> ptr;

        TcpClient(NetAddress::ptr addr, ProtocalType type = zRpcPb_Protocal);

        ~TcpClient();

        void init();

        void resetFd();

        int sendAndRecvZRpcPb(const std::string& msg_no, ZRpcPbStruct::pb_ptr& res);

        void stop();

        TcpConnection* getConnection();

        void setTimeout(const int v) {
            m_max_timeout = v;
        }

        void setTryCounts(const int v) {
            m_try_counts = v;
        }

        const std::string& getErrInfo() {
            return m_err_info;
        }

        NetAddress::ptr getPeerAddr() const {
            return m_peer_addr;
        }

        NetAddress::ptr getLocalAddr() const {
            return m_local_addr;
        }

        AbstractCodeC::ptr getCodeC() {
            return m_codec;
        }


    private:
        int m_family      = 0;
        int m_fd          = -1;
        int m_try_counts  = 3;           // max try reconnect times
        int m_max_timeout = 10000;       // max connect timeout, ms
        bool m_is_stop    = false;
        std::string m_err_info;         // error info of client

        NetAddress::ptr m_local_addr    = nullptr;
        NetAddress::ptr m_peer_addr     = nullptr;
        Reactor* m_reactor              = nullptr;
        TcpConnection::ptr m_connection = nullptr;
        AbstractCodeC::ptr m_codec      = nullptr;
        bool m_connect_success          = false;

    };
}

#endif //ZRPC_TCP_CLIENT_H
