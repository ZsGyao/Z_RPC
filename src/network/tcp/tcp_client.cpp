/**
  ******************************************************************************
  * @file           : tcp_client.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#include "src/network/tcp/tcp_client.h"
#include "src/common/log.h"
#include "src/network/http/http_codec.h"
#include "src/network/zRpcPb/zrpc_pb_codec.h"
#include "src/coroutine/hook.h"
#include "src/common/error_code.h"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


namespace zrpc {

    TcpClient::TcpClient(NetAddress::ptr addr, ProtocalType type) : m_peer_addr(addr) {
        m_family = m_peer_addr->getFamily();
        m_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_fd == -1) {
            ErrorLog << "call socket error, fd=-1, sys error=" << strerror(errno);
        }
        DebugLog << "TcpClient() create fd = " << m_fd;
        m_local_addr = std::make_shared<zrpc::IPAddress>("127.0.0.1", 0);
        m_reactor = Reactor::GetReactor();

        if (type == Http_Protocal) {
            m_codec = std::make_shared<HttpCodeC>();
        } else {
            m_codec = std::make_shared<ZRpcPbCodeC>();
        }

        m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128, m_peer_addr);
    }

    TcpClient::~TcpClient() {
        if (m_fd > 0) {
            FdEventContainer::GetFdContainer()->getFdEvent(m_fd)->unregisterFromReactor();
            close(m_fd);
            DebugLog << "~TcpClient() close fd = " << m_fd;
        }
    }

    TcpConnection* TcpClient::getConnection() {
        if (!m_connection.get()) {
            m_connection = std::make_shared<TcpConnection>(this, m_reactor, m_fd, 128, m_peer_addr);
        }
        return m_connection.get();
    }

    void TcpClient::resetFd() {
        zrpc::FdEvent::ptr fd_event = zrpc::FdEventContainer::GetFdContainer()->getFdEvent(m_fd);
        fd_event->unregisterFromReactor();
        close(m_fd);
        m_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_fd == -1) {
            ErrorLog << "call socket error, fd=-1, sys error=" << strerror(errno);
        }
    }

    int TcpClient::sendAndRecvZRpcPb(const std::string& msg_no, ZRpcPbStruct::pb_ptr& res) {
        bool is_timeout = false;
        zrpc::Coroutine::ptr cur_cor = zrpc::Coroutine::GetCurrentCoroutine();
        auto timer_cb = [this, &is_timeout, cur_cor]() {
            InfoLog << "TcpClient timer time out event occur";
            is_timeout = true;
            this->m_connection->setOverTimeFlag(true);
            cur_cor->resume();
        };
        TimerEvent::ptr event = std::make_shared<TimerEvent>(m_max_timeout, false, timer_cb);
        m_reactor->getTimer()->addTimerEvent(event);

        DebugLog << "add rpc timer event, timeout on " << event->getArriveTime();

        while (!is_timeout) {
            DebugLog << "begin to connect";
            if (m_connection->getState() != Connected) {
                int rt = connect_hook(m_fd, reinterpret_cast<sockaddr*>(m_peer_addr->getSockAddr()), m_peer_addr->getSockLen());
                if (rt == 0) {
                    DebugLog << "connect [" << m_peer_addr->toString() << "] success!";
                    m_connection->setUpClient();
                    break;
                }
                resetFd();
                if (is_timeout) {
                    InfoLog << "connect timeout, break";
                    goto err_deal;
                }
                if (errno == ECONNREFUSED) {
                    std::stringstream ss;
                    ss << "connect error, peer[ " << m_peer_addr->toString() <<  " ] closed.";
                    m_err_info = ss.str();
                    ErrorLog << "cancel overtime event, err info=" << m_err_info;
                    m_reactor->getTimer()->delTimerEvent(event);
                    return ERROR_PEER_CLOSED;
                }
                if (errno == EAFNOSUPPORT) {
                    std::stringstream ss;
                    ss << "connect cur sys error, err info is " << std::string(strerror(errno)) <<  " ] closed.";
                    m_err_info = ss.str();
                    ErrorLog << "cancel overtime event, err info=" << m_err_info;
                    m_reactor->getTimer()->delTimerEvent(event);
                    return ERROR_CONNECT_SYS_ERR;
                }
            } else {
                break;
            }
        }

        if (m_connection->getState() != Connected) {
            std::stringstream ss;
            ss << "connect peer addr[" << m_peer_addr->toString() << "] error. sys error=" << strerror(errno);
            m_err_info = ss.str();
            m_reactor->getTimer()->delTimerEvent(event);
            return ERROR_FAILED_CONNECT;
        }

        m_connection->setUpClient();
        m_connection->output();
        if (m_connection->getOverTimerFlag()) {
            InfoLog << "send data over time";
            is_timeout = true;
            goto err_deal;
        }

        while (!m_connection->getResPackageData(msg_no, res)) {
            DebugLog << "redo getResPackageData";
            m_connection->input();

            if (m_connection->getOverTimerFlag()) {
                InfoLog << "read data over time";
                is_timeout = true;
                goto err_deal;
            }
            if (m_connection->getState() == Closed) {
                InfoLog << "peer close";
                goto err_deal;
            }

            m_connection->execute();

        }

        m_reactor->getTimer()->delTimerEvent(event);
        m_err_info = "";
        return 0;

        err_deal:
        // connect error should close fd and reopen new one
        FdEventContainer::GetFdContainer()->getFdEvent(m_fd)->unregisterFromReactor();
        close(m_fd);
        m_fd = socket(AF_INET, SOCK_STREAM, 0);
        std::stringstream ss;
        if (is_timeout) {
            ss << "call rpc failed, over " << m_max_timeout << " ms";
            m_err_info = ss.str();

            m_connection->setOverTimeFlag(false);
            return ERROR_RPC_CALL_TIMEOUT;
        } else {
            ss << "call rpc failed, peer closed [" << m_peer_addr->toString() << "]";
            m_err_info = ss.str();
            return ERROR_PEER_CLOSED;
        }
    }

    void TcpClient::stop() {
        if (!m_is_stop) {
            m_is_stop = true;
            m_reactor->stop();
        }
    }
}