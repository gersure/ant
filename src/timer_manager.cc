//
// Created by zhengcf on 2019-06-26.
//

#include <thread>
#include <chrono>
#include <algorithm>
#include <exception>
#include <iostream>
#include "ant/timer_manager.hh"

namespace ant {

//timer_manager::TimerId const timer_manager::empty =
//        std::numeric_limits<timer_manager::TimerId>::max();

/** @struct timer
 *  @brief simple container to keep one timer
 */
struct timer {
    timer(timer_manager::TimerId p_id,
          timer_manager::Action const &&a,
          timer_manager::Action const &&ca)
            : id(p_id), timeout_action(std::move(a)),
              cancel_action(std::move(ca)) {};

    timer_manager::TimerId id;
    timer_manager::Action timeout_action;
    timer_manager::Action cancel_action;
};

timer_manager::timer_manager()
        : timeouts_(), last_timer_(0),
          timeouts_mutex_(), wait_condition_(),
          manager_mutex_(), is_stopping_(false) {
}

timer_manager::~timer_manager() {
}

timer_manager::TimerId timer_manager::add_timer(
        timer_manager::Timeout timeout,
        Action &&timeout_action) {
    return add_timer(timeout, std::move(timeout_action), 0);
}

timer_manager::TimerId
timer_manager::add_timer(timer_manager::Timeout timeout,
                         Action &&timeout_action,
                         Action const &&cancel_action) {
    std::lock_guard<std::mutex> accessGuard(timeouts_mutex_);

    timer_ptr p_timer(new timer(++last_timer_, std::move(timeout_action), std::move(cancel_action)));
    Timeout absolute_timeout = Clock::now().time_since_epoch() + timeout;
    TimeoutMap::iterator timer_it = timeouts_.insert(std::make_pair(absolute_timeout, p_timer));

    wait_condition_.notify_one();
    return timer_it->second->id;
}

namespace {

struct equal_id : std::unary_function<timer_manager::TimeoutMap::value_type, bool> {
    equal_id(timer_manager::TimerId id) : id_(id) {}

    bool operator()(timer_manager::TimeoutMap::value_type const &v) const {
        if (v.second) {
            return v.second->id == id_;
        }
        return false;
    }

    timer_manager::TimerId id_;
};

}

bool timer_manager::cancel_timer(timer_manager::TimerId id) {
    bool result = false;
    Action a;
    { // mutex scope
        std::lock_guard<std::mutex> accessGuard(timeouts_mutex_);

        TimeoutMap::iterator timer_it = find_if(timeouts_.begin(),
                                                timeouts_.end(),
                                                equal_id(id));

        if (timer_it != timeouts_.end()) {
            Action a = timer_it->second->cancel_action;
            timeouts_.erase(timer_it);
        } else {
            // log timer not found
        }
        result = true;
        wait_condition_.notify_one();
    }
    // run action after releasing lock to avoid deadlock if action works on timer_manager
    if (a) {
        a(id);
    }
    return result;
}

void timer_manager::stop() {
    std::lock_guard<std::mutex> accessGuard(manager_mutex_);
    is_stopping_ = true;
    wait_condition_.notify_one();
}

void timer_manager::operator()() {
    //using namespace boost::posix_time;
    using namespace std;
    for (;;) {
        { // check if timer manager is stopping
            std::lock_guard<std::mutex> stoppingLock(manager_mutex_);

            if (is_stopping_) {
                //cout << "Timer manager is stopping" << std::endl;
                /// @todo call all left timeouts to notify consumers that operation is closing
                return;
            }
        }

        TimeoutMap current_actions;
        { // opening scope for mutex
            std::lock_guard<std::mutex> accessGuard(timeouts_mutex_);
            Timeout timeout_now = Clock::now().time_since_epoch();
            // read current timeouts and execute actions for them
            TimeoutIterator match_time = timeouts_.upper_bound(timeout_now);
            current_actions.insert(timeouts_.begin(), match_time);
            timeouts_.erase(timeouts_.begin(), match_time);
        } // mutex is closed here so if running action is doing something on timer_manager it will nod deadlock
        run_actions(current_actions);
        { // opening scope for mutex
            std::unique_lock<std::mutex> accessGuard(timeouts_mutex_);
            /// @todo add check if time is not met for next timeout
            if (!timeouts_.empty()) {
                TimeoutIterator next_timeout_it = timeouts_.begin();
                Clock::time_point next_timeout_point(next_timeout_it->first);
                wait_condition_.wait_until(accessGuard, next_timeout_point);
                //std::cout << "timed_wait wakeup " << std::time(0) << std::endl;
            } else {
                wait_condition_.wait(accessGuard);
                //std::cout << "wait wakeup " << std::time(0) << std::endl;
            }
        }
    }
}

void timer_manager::run_actions(TimeoutMap const &actions) const {
    for (ConstTimeoutIterator action_it = actions.begin(); action_it != actions.end(); ++action_it) {
        Action a = action_it->second->timeout_action;
        if (a) {
            try {
                a(action_it->second->id);
            } catch (std::exception const &e) {
                // log error
                std::cerr << "Exception caught while running timer action: " << e.what() << std::endl;
            } catch (...) {
                // log error
                std::cerr << "Unknown exception caught";
            }
        }
    }
}

}