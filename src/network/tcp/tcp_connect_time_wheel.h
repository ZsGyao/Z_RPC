/**
  ******************************************************************************
  * @file           : tcp_connect_time_wheel.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-17
  ******************************************************************************
  */

#ifndef ZRPC_TCP_CONNECT_TIME_WHEEL_H
#define ZRPC_TCP_CONNECT_TIME_WHEEL_H

#include <memory>
#include "src/network/tcp/abstract_slot.h"
#include "src/network/reactor.h"
#include "src/network/timer.h"



namespace zrpc {

    extern std::shared_ptr<Config> zRpcConfig;

    class TcpConnection;

    class TcpTimeWheel {
        typedef std::shared_ptr<TcpTimeWheel> ptr;
        typedef AbstractSlot<TcpConnection> TcpConnectionSlot;

        explicit TcpTimeWheel(Reactor* reactor, int bucket_count = zRpcConfig->m_time_wheel_bucket_num,
                     int interval = zRpcConfig->m_time_wheel_bucket_num);

        ~TcpTimeWheel();

        void fresh(TcpConnectionSlot::ptr slot);

        void loopFunc();


    private:
        Reactor* m_reactor = nullptr;
        int m_bucket_count = 0;
        int m_interval     = 0;    // second

        TimerEvent::ptr m_event;
        std::queue<std::vector<TcpConnectionSlot::ptr>> m_wheel;

    };
}

#endif //ZRPC_TCP_CONNECT_TIME_WHEEL_H
