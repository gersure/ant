//
// Created by zhengcf on 2019-06-26.
//

#pragma once

#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <chrono>
#include <functional>
#include <condition_variable>
#include "ant/util/noncopyable.hh"

namespace ant {

struct timer;
using  timer_ptr = std::shared_ptr<timer> ;

class timer_manager : noncopyable {
public:
    using Clock   = std::chrono::system_clock;
    using TimerId = unsigned long;
    using Timeout = Clock::duration;
    using Action  = std::function<void(TimerId)>;

    /** @typedef std::multimap<Timeout, timer_ptr>  TimeoutMap
     * @brief map used to store each timeout mapped to it's action.
     * @todo probably this should be changed to use multi_index container or two separate containers to simplify searching for TimerId and for timeout.
     * @todo for now only timeout will be used as index simplifying to search and group of actions to execute.
     * @todo probably timers should be "rounded" when added to timer_manager to minimize wakeup count (e.g. 0.1 second timer groups)
     */

    using  TimeoutMap            = std::multimap<Timeout, timer_ptr>;
    using  TimeoutIterator       = TimeoutMap::iterator;
    using  ConstTimeoutIterator  = TimeoutMap::const_iterator;

    static TimerId const empty = std::numeric_limits<timer_manager::TimerId>::max();
public:
    timer_manager();

    ~timer_manager();

public:
    /**
     * @brief Add new timer with action to execute
     * @param timeout value
     * @param timeout_action to be executed when @a timeout is met
     * @return TimerId value which can be used to cancel timeout
     */
    TimerId add_timer(Timeout timeout, Action &&action);

    /**
     * @brief Add new timer with action to execute and action to execute in case of timer cancel
     * @param timeout value
     * @param timeout_action to be executed when @a timeout is met
     * @param cancel_action to be executed when @f cancel_timer called
     * @return TimerId value which can be used to cancel timeout
     */
    TimerId add_timer(Timeout timeout, Action &&action, Action const &&cancel_action);

    /**
     * @brief add new timer with action to execute
     * @param id timer id which should be cancelled
     * @return true if cancelling succeded
     */
    bool cancel_timer(TimerId id);

    void stop();    //!< stop timer manager thread

public:
    void operator()(); //!< thread execution function for timer manager

private:
    void run_actions(TimeoutMap const &actions) const; //!< run all actions within given container

private:
    TimeoutMap timeouts_;    //!< map storing all timeouts handled by manager
    TimerId    last_timer_;
    mutable std::mutex      timeouts_mutex_;
    std::condition_variable wait_condition_; //!< condition used by timer_manager thread to wait

    mutable std::mutex      manager_mutex_;    //!< mutex protecting internal timer_manager state
    bool is_stopping_;    //!< flag indicating that timer_manager is stopping
};

}