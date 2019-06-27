//
// Created by zhengcf on 2019-06-26.
//

#pragma once

namespace ant {

class noncopyable {
protected:
    noncopyable() = default;

    ~noncopyable() = default;

    noncopyable(const noncopyable &) = delete;

    noncopyable &operator=(const noncopyable &) = delete;
};

}