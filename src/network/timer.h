/**
  ******************************************************************************
  * @file           : timer.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-13
  ******************************************************************************
  */


#ifndef Z_RPC_TIMER_H
#define Z_RPC_TIMER_H

#include <memory>
#include <sys/time.h>
#include <functional>
#include "src/network/fd_event.h"
#include "src/network/reactor.h"
#include "src/network/mutex.hpp"

namespace zrpc {

    int64_t getNowTimeMs();

    class TimerEvent {
        friend class Timer;
    public:
        typedef std::shared_ptr<TimerEvent> ptr;

        /**
         * @param interval after interval ms, time_event execute
         * @param is_repeated whether time_event need to repeat add to Timer
         * @param task when time arrive, m_task execute
         */
        TimerEvent(int64_t interval, bool is_repeated, std::function<void()> task);

        ~TimerEvent();

        /**
         * @brief 重设定时事件的到达事件
         * @attention 定时时长不变，到达时间等于 getCurrentMs() + m_interval
         */
        void resetTime();

        void wake() {
            m_is_canceled = false;
        }

        void cancel() {
            m_is_canceled = true;
        }

        void cancelRepeated() {
            m_is_repeated = false;
        }

        int64_t getArriveTime() const {
            return m_arrive_time;
        }

    private:
        int64_t m_arrive_time;        // when to execute task, ms
        int64_t m_interval;           // interval between two tasks, ms
        bool m_is_repeated = false;   // whether time_event need to repeat add to Timer
        bool m_is_canceled = false;   // whether cancel time_event
        std::function<void()> m_task; // when time arrive, m_task execute
    };

    class Timer : public zrpc::FdEvent {
    public:
        typedef std::shared_ptr<Timer> ptr;

        /**
         * @brief Timer construct
         * @param reactor register the timer to reactor
         */
        explicit Timer(Reactor* reactor);

        ~Timer();

        void addTimerEvent(TimerEvent::ptr event, bool need_reset = true);

        void delTimerEvent(TimerEvent::ptr event);

        void resetArriveTime();

        void onTimer();

    private:
        /* first   ->  arrive_time
         * second  ->  timer_event */
        std::multimap<int64_t, TimerEvent::ptr> m_pending_events;
        RWMutex m_event_mutex;
    };

}

#endif //Z_RPC_TIMER_H
