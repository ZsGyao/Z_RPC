/**
  ******************************************************************************
  * @file           : timer.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-13
  ******************************************************************************
  */

#include "src/network/timer.h"
#include "src/common/log.h"
#include "src/coroutine/hook.h"
#include <sys/timerfd.h>
#include <iostream>
#include <string>

extern read_fun_ptr_t g_sys_read_fun;

namespace zrpc {

    int64_t getCurrentMs() {
        timeval val;
        gettimeofday(&val, nullptr);
        int64_t re = val.tv_sec * 1000 + val.tv_usec / 1000;
        return re;
    }

    TimerEvent::TimerEvent(int64_t interval, bool is_repeated, std::function<void()> task)
            : m_interval(interval),
              m_is_repeated(is_repeated),
              m_task(std::move(task)) {
        DebugLog << "TimerEvent construct [m_interval:"
                 << m_interval << "] [m_is_repeated:" << (bool)m_is_repeated << "]";

        m_arrive_time = getCurrentMs() + m_interval;               // time_event arrive time
        DebugLog << "time_event will occur at " << m_arrive_time;
    }

    TimerEvent::~TimerEvent() {}

    void TimerEvent::resetTime() {
        m_arrive_time = getCurrentMs() + m_interval;
        m_is_canceled = false;
    }

    Timer::Timer(Reactor* reactor) : zrpc::FdEvent(reactor) {
        m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC); // 创建定时器 fd
        DebugLog << "call timerfd_create(), timer_fd = " << m_fd;
        if (m_fd == -1) {
            ErrorLog << "timerfd_create() error";
        }
        /*  timer_fd listen read_event, when read_event arrive, execute this.onTimer() */
        m_read_callback = std::bind(&Timer::onTimer, this);
        /* timer_fd listen read_event, and register the event to reactor,
         * when read_event arrive, reactor will return and execute m_read_callback -> Timer::onTimer */
        addListenEvents(READ);
    }

    Timer::~Timer() {
        /* reactor unregister the timer_fd  */
        unregisterFromReactor();
        close(m_fd);
    }

    void Timer::addTimerEvent(TimerEvent::ptr event, bool need_reset) {
        DebugLog << "Timer::addTimerEvent [event:" << event << "] [arrive_time:" << event->m_arrive_time << "]";

        RWMutex::WriteLock lock(m_event_mutex);
        bool is_reset = false;
        /* 1. if no timer_event in Timer, a timer_event is added, Timer need reset
         * 2. if the earliest arrival time of timer_event in Timer bigger than added event, Timer need reset */
        if (m_pending_events.empty()) {
            is_reset = true;
        } else {
            auto it = m_pending_events.begin();
            if (event->m_arrive_time < (*it).second->m_arrive_time) {
                is_reset = true;
            }
        }
        m_pending_events.emplace(event->m_arrive_time, event);
        lock.unlock();

        if (is_reset && need_reset) {
            DebugLog << "The new timer_event with earlier arrival time is added, Timer need reset";
            resetArriveTime();
        }
    }

    /* no need for resetArrivalTime, because in onTimer when event->m_is_canceled = true , m_task in timer_event do not execute */
    void Timer::delTimerEvent(TimerEvent::ptr event) {
        /* event si canceled from Timer */
        event->m_is_canceled = true;
        RWMutex::WriteLock lock(m_event_mutex);
        /* Find the lower_bound and upper_bound of timer_event as event->m_ arrive_timer in Timer pending timer_event
         * select deletion event and erase */
        auto begin = m_pending_events.lower_bound(event->m_arrive_time);
        auto end = m_pending_events.upper_bound(event->m_arrive_time);
        auto it = begin;
        for (it = begin; it != end; it++) {
            if (it->second == event) {
                DebugLog << "Find deletion timer event, now delete it. src arrive time=" << event->m_arrive_time;
                break;
            }
        }
        if (it != m_pending_events.end()) {
            m_pending_events.erase(it);
        } else {
            WarnLog << "delTimerEvent: have no event [" << event << "] arrive time [" << event->m_arrive_time << "] in Timer pending events";
        }
        lock.unlock();
        DebugLog << "del timer event success, origin arrive time=" << event->m_arrive_time;
    }

    void Timer::resetArriveTime() {
        RWMutex::ReadLock lock(m_event_mutex);
        std::multimap<int64_t, TimerEvent::ptr> tmp = m_pending_events;
        lock.unlock();

        if (tmp.empty()) {
            DebugLog << "Timer: no timer event pending, pending_events empty";
            return;
        }
        /* resetArriveTime() will be called when new timer_event added, need to check whether expire */
        int64_t now = getCurrentMs();
        auto it = tmp.begin();
        if ((*it).first < now) {
            WarnLog << "New added timer events has already expired";
            return;
        }

        int64_t interval = (*it).first - now;
        itimerspec new_value;
        memset(&new_value, 0, sizeof(new_value));

        timespec ts;
        memset(&ts, 0, sizeof(ts));
        ts.tv_sec = interval / 1000;
        ts.tv_nsec = (interval % 1000) * 1000000;
        new_value.it_value = ts;

        int rt = timerfd_settime(m_fd, 0, &new_value, nullptr);
        if (rt != 0) {
            ErrorLog << "tiemrfd_settime error, interval=" << interval;
        } else {
             DebugLog << "Timer resetArriveTime success, next occur time = " << (*it).first;
        }
    }

    void Timer::onTimer() {
        DebugLog << "onTimer execute, read event in [timerfd:" << m_fd << "] arrive" ;
        char buf[8];
        while(true) {
            if((g_sys_read_fun(m_fd, buf, 8) == -1) && errno == EAGAIN) {
                break;
            }
        }

        int64_t now = getCurrentMs();
        RWMutex::WriteLock lock(m_event_mutex);
        auto it = m_pending_events.begin();
        std::vector<TimerEvent::ptr> tmps;
        std::vector<std::pair<int64_t, std::function<void()>>> tasks;

        for(it = m_pending_events.begin(); it != m_pending_events.end(); it++) {
            if((*it).first <= now && !(*it).second->m_is_canceled) {
                tmps.push_back((*it).second);
                tasks.emplace_back(std::make_pair((*it).second->m_arrive_time, (*it).second->m_task));
            } else {
                break;
            }
        }
        m_pending_events.erase(m_pending_events.begin(), it);
        lock.unlock();
        for (const auto& timer_event: tmps) {
             DebugLog << "timer_event [" << timer_event << "] is repeated, add to Timer again";
            if (timer_event->m_is_repeated) {
                timer_event->resetTime();
                addTimerEvent(timer_event, false);
            }
        }
        resetArriveTime();

        for (const auto& task : tasks) {
            DebugLog << "Timer execute timer event:" << task.first;
            task.second();
        }
    }

}