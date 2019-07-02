//
// Created by zhengcf on 2019-06-28.
//

#pragma once

#include "ant/util/noncopyable.hh"
#include "ant/lowres_clock.hh"
#include "ant/manual_clock.hh"
#include "ant/timer.hh"


namespace ant {

class timer_manager final : public noncopyable {
public:


private:
    timer_set<timer<>, &timer<>::_link> _timers;
    timer_set<timer<>, &timer<>::_link>::timer_list_t _expired_timers;
    timer_set<timer<lowres_clock>, &timer<lowres_clock>::_link> _lowres_timers;
    timer_set<timer<lowres_clock>, &timer<lowres_clock>::_link>::timer_list_t _expired_lowres_timers;
    timer_set<timer<manual_clock>, &timer<manual_clock>::_link> _manual_timers;
    timer_set<timer<manual_clock>, &timer<manual_clock>::_link>::timer_list_t _expired_manual_timers;

};




}