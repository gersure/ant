//
// Created by zhengcf on 2019-06-27.
//


#include <iostream>
#include <sstream>
#include <memory>
#include <functional>
#include <thread>

#include "ant/lowres_timer_manager.hh"

using namespace ant;

using std::chrono::seconds;
using std::chrono::microseconds;
using std::placeholders::_1;

/** simple timer action to print value on console */
struct TimePrint : std::binary_function<lowres_timer_manager::TimerId, std::string, void> {
    void operator()(lowres_timer_manager::TimerId id, std::string const &message) const {
        std::cout << message << ", id=" << id << std::endl;
    }
};

/** simple timer action to extend its timer */
struct SelfExtend : std::unary_function<lowres_timer_manager::TimerId, void>{
    SelfExtend(std::shared_ptr <lowres_timer_manager> manager)
            : manager_(manager) {}

    void operator()(lowres_timer_manager::TimerId id) const {
        std::ostringstream ssMessage;
        ssMessage << "timer " << id;
        std::shared_ptr<lowres_timer_manager> manager = manager_.lock();
        if (manager) {
            lowres_timer_manager::TimerId new_id = manager->add_timer(seconds(10), SelfExtend(manager));
            ssMessage << ", extended timer, new_id(" << new_id << ")" << std::endl;
        } else {
            ssMessage << ", Error manager pointer invalid" << std::endl;
        }
        std::cout << ssMessage.str();
    }

private:
    std::weak_ptr<lowres_timer_manager> manager_;
};

/** timer action which will execute stop call on timer manager to stop timer work */
struct TimerManagerFinish {
    TimerManagerFinish(std::shared_ptr <lowres_timer_manager> manager)
            : manager_(manager) {}

    void operator()(lowres_timer_manager::TimerId id) const {
        std::ostringstream ssMessage;
        ssMessage << "timer " << id;
        std::shared_ptr<lowres_timer_manager> manager = manager_.lock();
        if (manager) {
            manager->stop();
            ssMessage << ", send stop message to timer manager" << std::endl;
        } else {
            ssMessage << ", wrong timer manager pointer" << std::endl;
        }
        std::cout << ssMessage.str();
    }

private:
    std::weak_ptr<lowres_timer_manager> manager_;
};

/**
 * @struct timer_manager_deleter
 * @brief Simple deleter which can be used with std::shared_ptr storing
 * lowres_timer_manager instance. Before deleting object it will call stop on timer
 * manager object to gracefully finish if it is executed by thread.
 *
 */
struct timer_manager_deleter : std::unary_function<lowres_timer_manager *, void> {
    timer_manager_deleter() : thread_() {};

    void operator()(lowres_timer_manager *t) const {
        using std::cout;
        if (t) {
            t->stop();
            std::shared_ptr <std::thread> th = thread_.lock();
            if (th) {
                cout << "Joining manager thread" << std::endl;
                th->join();
            } else {
                cout << "sleeping before deletion" << std::endl;
                // sleep a little while to allow thead complete
                std::this_thread::sleep_for(seconds(2));
            }
            delete t;
        }
    }

    void setThread(std::shared_ptr<std::thread> th) {
        thread_ = th;
    }

    std::weak_ptr<std::thread> thread_;
};

int main(int argc, char **argv) {
    using namespace std;
    // declare new lowres_timer_manager
    std::shared_ptr<std::thread> manager_thread;
    std::shared_ptr<lowres_timer_manager> manager(new lowres_timer_manager, timer_manager_deleter());

    manager->add_timer(seconds(1), std::bind(TimePrint(), _1, "timeout 1s"));
    manager->add_timer(seconds(2), std::bind(TimePrint(), _1, "timeout 2s no cancel"),
                       std::bind(TimePrint(), _1, "cancel"));
    lowres_timer_manager::TimerId cancel_id = manager->add_timer(seconds(5), std::bind(TimePrint(),_1, "timeout 5s cancel"),
                                                          std::bind(TimePrint(), _1, "cancel 5s cancel"));

    manager_thread.reset(new std::thread(std::ref(*manager)));
    if (timer_manager_deleter *d = std::get_deleter<timer_manager_deleter>(manager)) {
        d->setThread(manager_thread);
    }
    for (int i = 0; i < 5; ++i) {
        manager->add_timer(seconds(8), std::bind(TimePrint(), _1, " multiple timeout timeouts at the same time"));
    }
    manager->add_timer(seconds(3), std::bind(TimePrint(), _1, "timeout 3s"));
    manager->add_timer(seconds(6), std::bind(TimePrint(), _1, "timeout 6s"));
    manager->add_timer(microseconds(9), std::bind(TimePrint(), _1, "timeout 9ms"));
    manager->add_timer(seconds(10), std::bind(TimePrint(), _1, "timeout 10s"));
    manager->add_timer(seconds(10), SelfExtend(manager));
    manager->add_timer(seconds(60), TimerManagerFinish(manager));
    std::this_thread::sleep_for(seconds(3));

    cout << "cancelling timer id:" << cancel_id << std::endl;
    manager->cancel_timer(cancel_id);
    manager->add_timer(seconds(2), std::bind(TimePrint(), _1, "timeout 2s"));

    for (int i = 0; i < 20000; ++i) {
//        cout << "main thread iteration: " << i << std::endl;
        std::this_thread::sleep_for(seconds(1));
    }
    if(timer_manager_deleter* d=std::get_deleter<timer_manager_deleter>(manager)) {
    	d->setThread(0);
    }
    manager.reset();
    cout << "Joining manager thread" << std::endl;
    manager_thread->join();
    cout << "Test programs finished" << std::endl;
    return 0;
}