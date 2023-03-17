/**
  ******************************************************************************
  * @file           : tcp_connect_timer.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-17
  ******************************************************************************
  */

#include "src/network/tcp/tcp_connect_time_wheel.h"
#include "src/common/log.h"

namespace zrpc {

    TcpTimeWheel::TcpTimeWheel(Reactor* reactor, int bucket_count, int interval)
            : m_reactor(reactor),
              m_bucket_count (bucket_count),
              m_interval(interval) {
        for(int i = 0; i < bucket_count; i++) {
            std::vector<TcpConnectionSlot::ptr> tmp;
            m_wheel.push(tmp);
        }
        m_event = std::make_shared<TimerEvent>(m_interval * 1000, true, std::bind(&TcpTimeWheel::loopFunc, this));
        m_reactor->getTimer()->delTimerEvent(m_event);
    }

    TcpTimeWheel::~TcpTimeWheel() {
        m_reactor->getTimer()->delTimerEvent(m_event);
    }

    /* when tcp connect be used, push it to the back of wheel, otherwise we think it overtime and delete it */
    void TcpTimeWheel::fresh(TcpConnectionSlot::ptr slot) {
        DebugLog << "fresh connection";
        m_wheel.back().emplace_back(slot);
    }

    void TcpTimeWheel::loopFunc() {
        m_wheel.pop();
        std::vector<TcpConnectionSlot::ptr> tmp;
        m_wheel.push(tmp);
        DebugLog << "TcpConnect overtime, pop and push new slot";
    }

}