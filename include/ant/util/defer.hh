//
// Created by didi on 2019-08-23.
//

#ifndef ANT_DEFER_HH
#define ANT_DEFER_HH

#pragma once

#include <type_traits>

namespace ant {

    template<typename Func>
    class deferred_action {
        Func _func;
        bool _cancelled = false;
    public:
        static_assert(std::is_nothrow_move_constructible<Func>::value, "Func(Func&&) must be noexcept");

        deferred_action(Func &&func) noexcept : _func(std::move(func)) {}

        deferred_action(deferred_action &&o) noexcept : _func(std::move(o._func)), _cancelled(o._cancelled) {
            o._cancelled = true;
        }

        deferred_action &operator=(deferred_action &&o) noexcept {
            if (this != &o) {
                this->~deferred_action();
                new(this) deferred_action(std::move(o));
            }
            return *this;
        }

        deferred_action(const deferred_action &) = delete;

        ~deferred_action() { if (!_cancelled) { _func(); }; }

        void cancel() { _cancelled = true; }
    };

    template<typename Func>
    inline
    deferred_action<Func>
    defer(Func &&func) {
        return deferred_action<Func>(std::forward<Func>(func));
    }

}

#endif //ANT_DEFER_HH
