//
// Created by zhengcf on 2019-06-28.
//

#include "ant/lowres_clock.hh"
#include "ant/manual_clock.hh"
#include "ant/timer.hh"
#include "ant/timer_manager.hh"

namespace ant {

    std::atomic<lowres_clock_impl::steady_rep> lowres_clock_impl::counters::_steady_now;
    std::atomic<lowres_clock_impl::system_rep> lowres_clock_impl::counters::_system_now;
    std::atomic<manual_clock::rep> manual_clock::_now;
    constexpr std::chrono::milliseconds lowres_clock_impl::_granularity;


    timespec to_timespec(steady_clock_type::time_point t) {
        using ns = std::chrono::nanoseconds;
        auto n = std::chrono::duration_cast<ns>(t.time_since_epoch()).count();
        return {n / 1000000000, n % 1000000000};
    }

    lowres_clock_impl::lowres_clock_impl() {
        update();
        _timer.set_callback(&lowres_clock_impl::update);
        _timer.arm_periodic(_granularity);
    }

    void lowres_clock_impl::update() {
        auto const steady_count =
                std::chrono::duration_cast<steady_duration>(base_steady_clock::now().time_since_epoch()).count();

        auto const system_count =
                std::chrono::duration_cast<system_duration>(base_system_clock::now().time_since_epoch()).count();

        counters::_steady_now.store(steady_count, std::memory_order_relaxed);
        counters::_system_now.store(system_count, std::memory_order_relaxed);
    }

    template<typename Clock>
    inline
    timer<Clock>::~timer() {
        if (_queued) {
            engine().del_timer(this);
        }
    }

    template<typename Clock>
    inline
    void timer<Clock>::arm(time_point until, compat::optional<duration> period) {
        arm_state(until, period);
        engine().add_timer(this);
    }

    template<typename Clock>
    inline
    void timer<Clock>::readd_periodic() {
        arm_state(Clock::now() + _period.value(), {_period.value()});
        engine().queue_timer(this);
    }

    template<typename Clock>
    inline
    bool timer<Clock>::cancel() {
        if (!_armed) {
            return false;
        }
        _armed = false;
        if (_queued) {
            engine().del_timer(this);
            _queued = false;
        }
        return true;
    }

    template
    class timer<steady_clock_type>;

    template
    class timer<lowres_clock>;

    template
    class timer<manual_clock>;


    void timer_manager::add_timer(timer<steady_clock_type> *tmr) {
        if (queue_timer(tmr)) {
            enable_timer(_timers.get_next_timeout());
        }
    }

    bool timer_manager::queue_timer(timer<steady_clock_type> *tmr) {
        return _timers.insert(*tmr);
    }

    void timer_manager::del_timer(timer<steady_clock_type> *tmr) {
        if (tmr->_expired) {
            _expired_timers.erase(_expired_timers.iterator_to(*tmr));
            tmr->_expired = false;
        } else {
            _timers.remove(*tmr);
        }
    }

    void timer_manager::add_timer(timer<lowres_clock> *tmr) {
        if (queue_timer(tmr)) {
            _lowres_next_timeout = _lowres_timers.get_next_timeout();
        }
    }

    bool timer_manager::queue_timer(timer<lowres_clock> *tmr) {
        return _lowres_timers.insert(*tmr);
    }

    void timer_manager::del_timer(timer<lowres_clock> *tmr) {
        if (tmr->_expired) {
            _expired_lowres_timers.erase(_expired_lowres_timers.iterator_to(*tmr));
            tmr->_expired = false;
        } else {
            _lowres_timers.remove(*tmr);
        }
    }

    void timer_manager::add_timer(timer<manual_clock> *tmr) {
        queue_timer(tmr);
    }

    bool timer_manager::queue_timer(timer<manual_clock> *tmr) {
        return _manual_timers.insert(*tmr);
    }

    void timer_manager::del_timer(timer<manual_clock> *tmr) {
        if (tmr->_expired) {
            _expired_manual_timers.erase(_expired_manual_timers.iterator_to(*tmr));
            tmr->_expired = false;
        } else {
            _manual_timers.remove(*tmr);
        }
    }


    void timer_manager::enable_timer(steady_clock_type::time_point when) {
        itimerspec its;
        its.it_interval = {};
        its.it_value = to_timespec(when);
        arm_highres_timer(its);
    }
}