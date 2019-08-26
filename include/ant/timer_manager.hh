//
// Created by zhengcf on 2019-06-28.
//

#pragma once

#include "ant/util/noncopyable.hh"
#include "ant/lowres_clock.hh"
#include "ant/manual_clock.hh"
#include "ant/timer.hh"
#include "ant/posix.hh"
#include <chrono>


namespace ant {

class timer_manager final : public noncopyable {
public:

    void add_timer(timer<steady_clock_type>*);
    bool queue_timer(timer<steady_clock_type>*);
    void del_timer(timer<steady_clock_type>*);
    void add_timer(timer<lowres_clock>*);
    bool queue_timer(timer<lowres_clock>*);
    void del_timer(timer<lowres_clock>*);
    void add_timer(timer<manual_clock>*);
    bool queue_timer(timer<manual_clock>*);
    void del_timer(timer<manual_clock>*);

    void enable_timer(steady_clock_type::time_point when);

private:

    void arm_highres_timer(const ::itimerspec& its) {
        _steady_clock_timer.timerfd_settime(TFD_TIMER_ABSTIME, its);
    }

    static file_desc make_timerfd() {
        return file_desc::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK);
    }

    file_desc _steady_clock_timer = make_timerfd();
    lowres_clock::time_point _lowres_next_timeout;

    timer_set<timer<>, &timer<>::_link> _timers;
    timer_set<timer<>, &timer<>::_link>::timer_list_t _expired_timers;
    timer_set<timer<lowres_clock>, &timer<lowres_clock>::_link> _lowres_timers;
    timer_set<timer<lowres_clock>, &timer<lowres_clock>::_link>::timer_list_t _expired_lowres_timers;
    timer_set<timer<manual_clock>, &timer<manual_clock>::_link> _manual_timers;
    timer_set<timer<manual_clock>, &timer<manual_clock>::_link>::timer_list_t _expired_manual_timers;

};


extern __thread timer_manager local_timer_manager;

inline timer_manager& engine() {
    return local_timer_manager;
}



}