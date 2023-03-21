/**
  ******************************************************************************
  * @file           : tcp_connection.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-21
  ******************************************************************************
  */

#include "src/common/log.h"
#include "src/network/tcp/tcp_connection.h"
#include "src/network/tcp/tcp_server.h"
#include "src/network/tcp/tcp_client.h"
#include "src/network/tcp/tcp_buffer.h"
#include "src/coroutine/coroutine_pool.h"
#include "src/coroutine/hook.h"

namespace zrpc {
    TcpConnection::TcpConnection(zrpc::TcpServer* tcp_svr, zrpc::IOThread* io_thread,
                                 int fd, int buff_size, NetAddress::ptr peer_addr)
                : m_io_thread(io_thread),
                  m_fd(fd),
                  m_state(Connected),
                  m_connection_type(ServerConnection),
                  m_peer_addr(peer_addr) {
        m_reactor  = m_io_thread->getReactor();
        m_tcp_svr  = tcp_svr;
        m_codec    = m_tcp_svr->getCodeC();
        m_fd_event = FdEventContainer::GetFdContainer()->getFdEvent(fd);
        m_fd_event->setReactor(m_reactor);
        initBuffer(buff_size);
        m_loop_cor = zRpcCoroutinePool->getCoroutineInstance();
        m_state    = Connected;
        DebugLog << "success create tcp connection[" << m_state << "], fd=" << fd;
    }

    TcpConnection::TcpConnection(zrpc::TcpClient* tcp_cli, zrpc::Reactor* reactor,
                                 int fd, int buff_size, NetAddress::ptr peer_addr)
              : m_fd(fd),
                m_state(NotConnected),
                m_connection_type(ClientConnection),
                m_peer_addr(peer_addr){
        m_reactor = reactor;
        m_tcp_cli = tcp_cli;
        m_codec   = m_tcp_cli->getCodeC();
        m_fd_event = FdEventContainer::GetFdContainer()->getFdEvent(fd);
        m_fd_event->setReactor(m_reactor);
        initBuffer(buff_size);

        DebugLog << "success create tcp connection[NotConnected]";
    }

    void TcpConnection::setUpClient() {
        setState(Connected);
    }

    void TcpConnection::setUpServer() {
        m_reactor->addCoroutine(m_loop_cor);
    }

    TcpConnection::~TcpConnection() {
        if (m_connection_type == ServerConnection) {
            zRpcCoroutinePool->returnCoroutine(m_loop_cor);
        }
        DebugLog << "~TcpConnection, fd=" << m_fd;
    }

    void TcpConnection::initBuffer(int size) {
        // 初始化缓冲区大小
        m_write_buffer = std::make_shared<TcpBuffer>(size);
        m_read_buffer = std::make_shared<TcpBuffer>(size);
    }

    void TcpConnection::shutdownConnection() {
        TcpConnectionState state = getState();
        if (state == Closed || state == NotConnected) {
            DebugLog << "this client has closed";
            return;
        }
        setState(HalfClosing);
        InfoLog << "shutdown conn[" << m_peer_addr->toString() << "], fd=" << m_fd;
        // call sys shutdown to send FIN
        // wait client done something, client will send FIN
        // and fd occur read event but byte count is 0
        // then will call clearClient to set CLOSED
        // IOThread::MainLoopTimerFunc will delete CLOSED connection
        shutdown(m_fd_event->getFd(), SHUT_RDWR);
    }

    TcpConnectionState TcpConnection::getState() {
        TcpConnectionState state;
        RWMutex::ReadLock lock(m_mutex);
        state = m_state;
        lock.unlock();

        return state;
    }

    void TcpConnection::setState(const TcpConnectionState& state) {
        RWMutex::WriteLock lock(m_mutex);
        m_state = state;
        lock.unlock();
    }

    TcpBuffer* TcpConnection::getInBuffer() {
        return m_read_buffer.get();
    }

    TcpBuffer* TcpConnection::getOutBuffer() {
        return m_write_buffer.get();
    }

    AbstractCodeC::ptr TcpConnection::getCodec() const {
        return m_codec;
    }

    bool TcpConnection::getResPackageData(const std::string& msg_req, ZRpcPbStruct::pb_ptr& pb_struct) {
        auto it = m_reply_datas.find(msg_req);
        if (it != m_reply_datas.end()) {
            DebugLog << "return a request data";
            pb_struct = it->second;
            m_reply_datas.erase(it);
            return true;
        }
        DebugLog << msg_req << "|reply data not exist";
        return false;
    }

    void TcpConnection::registerToTimeWheel() {
        auto cb = [] (TcpConnection::ptr conn) {
            conn->shutdownConnection();
        };
        TcpTimeWheel::TcpConnectionSlot::ptr tmp = std::make_shared<AbstractSlot<TcpConnection>>(shared_from_this(), cb);
        m_weak_slot = tmp;
        m_tcp_svr->freshTcpConnection(tmp);
    }

    Coroutine::ptr TcpConnection::getCoroutine() {
        return m_loop_cor;
    }

    void TcpConnection::MainServerLoopCorFunc() {
        while (!m_stop) {
            input();

            execute();

            output();
        }
        InfoLog << "this connection has already end loop";
    }

    void TcpConnection::input() {
        if (m_is_over_time) {
            InfoLog << "over timer, skip input progress";
            return;
        }
        TcpConnectionState state = getState();
        if (state == Closed || state == NotConnected) {
            return;
        }
        bool read_all = false;
        bool close_flag = false;
        int count = 0;
        while (!read_all) {
            if (m_read_buffer->writeAble() == 0) {
                m_read_buffer->resizeBuffer(2 * m_read_buffer->getSize());
            }

            int read_count = m_read_buffer->writeAble();
            int write_index = m_read_buffer->writeIndex();

            DebugLog << "m_read_buffer size=" << m_read_buffer->getBufferVector().size() << "rd=" << m_read_buffer->readIndex() << "wd=" << m_read_buffer->writeIndex();
            int rt = read_hook(m_fd, &(m_read_buffer->m_buffer[write_index]), read_count);
            if (rt > 0) {
                m_read_buffer->recycleWrite(rt);
            }
            DebugLog << "m_read_buffer size=" << m_read_buffer->getBufferVector().size() << "rd=" << m_read_buffer->readIndex() << "wd=" << m_read_buffer->writeIndex();

            DebugLog << "read data back, fd=" << m_fd;
            count += rt;
            if (m_is_over_time) {
                InfoLog << "over timer, now break read function";
                break;
            }
            if (rt <= 0) {
                DebugLog << "rt <= 0";
                ErrorLog << "read empty while occur read event, because of peer close, fd= " << m_fd << ", sys error=" << strerror(errno) << ", now to clear tcp connection";
                // this cor can destroy
                close_flag = true;
                break;
            } else {
                if (rt == read_count) {
                    DebugLog << "read_count == rt";
                    // is is possible read more data, should continue read
                    continue;
                } else if (rt < read_count) {
                    DebugLog << "read_count > rt";
                    // read all data in socket buffer, skip out loop
                    read_all = true;
                    break;
                }
            }
        }
        if (close_flag) {
            clearClient();
            DebugLog << "peer close, now yield current coroutine, wait main thread clear this TcpConnection";
            Coroutine::GetCurrentCoroutine()->yield();
            // return;
        }

        if (m_is_over_time) {
            return;
        }

        if (!read_all) {
            ErrorLog << "not read all data in socket buffer";
        }
        InfoLog << "recv [" << count << "] bytes data from [" << m_peer_addr->toString() << "], fd [" << m_fd << "]";
        if (m_connection_type == ServerConnection) {
            TcpTimeWheel::TcpConnectionSlot::ptr tmp = m_weak_slot.lock();
            if (tmp) {
                m_tcp_svr->freshTcpConnection(tmp);
            }
        }
    }

    void TcpConnection::execute() {
        // it only server do this
        while(m_read_buffer->readAble() > 0) {
            std::shared_ptr<AbstractData> data;
            if (m_codec->getProtocalType() == zRpcPb_Protocal) {
                data = std::make_shared<ZRpcPbStruct>();
            } else {
                data = std::make_shared<HttpRequest>();
            }

            m_codec->decode(m_read_buffer.get(), data.get());
            // DebugLog << "parse service_name=" << pb_struct.service_full_name;
            if (!data->decode_success) {
                ErrorLog << "it parse request error of fd " << m_fd;
                break;
            }
            // DebugLog << "it parse request success";
            if (m_connection_type == ServerConnection) {
                // DebugLog << "to dispatch this package";
                m_tcp_svr->getDispatcher()->dispatch(data.get(), this);
                // DebugLog << "continue parse next package";
            } else if (m_connection_type == ClientConnection) {
                // TODO:
                std::shared_ptr<ZRpcPbStruct> tmp = std::dynamic_pointer_cast<ZRpcPbStruct>(data);
                if (tmp) {
                    m_reply_datas.insert(std::make_pair(tmp->msg_req, tmp));
                }
            }
        }
    }

    void TcpConnection::output() {
        if (m_is_over_time) {
            InfoLog << "over timer, skip output progress";
            return;
        }
        while(true) {
            TcpConnectionState state = getState();
            if (state != Connected) {
                break;
            }

            if (m_write_buffer->readAble() == 0) {
                DebugLog << "app buffer of fd[" << m_fd << "] no data to write, to yield this coroutine";
                break;
            }

            int total_size = m_write_buffer->readAble();
            int read_index = m_write_buffer->readIndex();
            int rt = write_hook(m_fd, &(m_write_buffer->m_buffer[read_index]), total_size);
            // InfoLog << "write end";
            if (rt <= 0) {
                ErrorLog << "write empty, error=" << strerror(errno);
            }

            DebugLog << "success write " << rt << " bytes";
            m_write_buffer->recycleRead(rt);
            DebugLog << "recycle write index =" << m_write_buffer->writeIndex() << ", read_index =" << m_write_buffer->readIndex() << "readable = " << m_write_buffer->readAble();
            InfoLog << "send[" << rt << "] bytes data to [" << m_peer_addr->toString() << "], fd [" << m_fd << "]";
            if (m_write_buffer->readAble() <= 0) {
                // InfoLog << "send all data, now unregister write event on reactor and yield Coroutine";
                InfoLog << "send all data, now unregister write event and break";
                // m_fd_event->delListenEvents(IOEvent::WRITE);
                break;
            }

            if (m_is_over_time) {
                InfoLog << "over timer, now break write function";
                break;
            }
        }
    }

    void TcpConnection::setOverTimeFlag(bool value) {
        m_is_over_time = value;
    }

    bool TcpConnection::getOverTimerFlag() {
        return m_is_over_time;
    }

    void TcpConnection::initServer() {
        registerToTimeWheel();
        m_loop_cor->resetCallBack(std::bind(&TcpConnection::MainServerLoopCorFunc, this));
    }

    void TcpConnection::clearClient() {
        if (getState() == Closed) {
            DebugLog << "this client has closed";
            return;
        }
        // first unregister epoll event
        m_fd_event->unregisterFromReactor();

        // stop read and write cor
        m_stop = true;

        close(m_fd_event->getFd());
        setState(Closed);
    }
}

