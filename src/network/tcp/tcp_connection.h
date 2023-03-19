/**
  ******************************************************************************
  * @file           : tcp_connect.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-19
  ******************************************************************************
  */

#ifndef ZRPC_TCP_CONNECTION_H
#define ZRPC_TCP_CONNECTION_H

#include <memory>
#include "src/network/tcp/io_thread.h"
#include "src/network/net_address.h"
#include "src/network/reactor.h"
#include "src/network/tcp/tcp_buffer.h"
#include "src/network/abstract_codec.h"

namespace zrpc {

    class TcpServer;
    class TcpClient;

    enum TcpConnectionState {
        NotConnected = 1,        // can do io
        Connected    = 2,        // can do io
        HalfClosing  = 3,        // server call shutdown, write half close. can read, but can't write
        Closed       = 4,        // can't do io
    };

    class TcpConnection : public std::shared_ptr<TcpConnection> {
    public:
        typedef std::shared_ptr<TcpConnection> ptr;

        TcpConnection(zrpc::TcpServer* tcp_svr,  zrpc::IOThread* io_thread, int fd, int buff_size, NetAddress::ptr peer_addr);

        TcpConnection(zrpc::TcpClient* tcp_cli, zrpc::Reactor* reactor, int fd, int buff_size, NetAddress::ptr peer_addr);

        void setUpClient();

        void setUpServer();

        ~TcpConnection();

        void initBuffer(int size);

        enum ConnectionType {
            ServerConnection = 1,     // owned by tcp_server
            ClientConnection = 2,     // owned by tcp_client
        };

    public:
        void shutdownConnection();

        TcpConnectionState getState();

        void setState(const TcpConnectionState& state);

        TcpBuffer* getInBuffer();

        TcpBuffer* getOutBuffer();

        AbstractCodeC::ptr getCodec() const;

        bool getResPackageData(const std::string& msg_req, zRpcPbStruct::pb_ptr& pb_struct);

        void registerToTimeWheel();

        Coroutine::ptr getCoroutine();

    public:
        void MainServerLoopCorFunc();

        void input();

        void execute();

        void output();

        void setOverTimeFlag(bool value);

        bool getOverTimerFlag();

        void initServer();

    private:
        void clearClient();

    private:
        TcpServer* m_tcp_svr {nullptr};
        TcpClient* m_tcp_cli {nullptr};
        IOThread* m_io_thread {nullptr};
        Reactor* m_reactor {nullptr};

        int m_fd {-1};
        TcpConnectionState m_state {TcpConnectionState::Connected};
        ConnectionType m_connection_type {ServerConnection};

        NetAddress::ptr m_peer_addr;


        TcpBuffer::ptr m_read_buffer;
        TcpBuffer::ptr m_write_buffer;

        Coroutine::ptr m_loop_cor;

        AbstractCodeC::ptr m_codec;

        FdEvent::ptr m_fd_event;

        bool m_stop {false};

        bool m_is_over_time {false};

        std::map<std::string, std::shared_ptr<TinyPbStruct>> m_reply_datas;

        std::weak_ptr<AbstractSlot<TcpConnection>> m_weak_slot;

        RWMutex m_mutex;
    };
}

#endif //ZRPC_TCP_CONNECTION_H
