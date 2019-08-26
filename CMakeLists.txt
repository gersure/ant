cmake_minimum_required(VERSION 3.0.2)
project(ant)

set(CMAKE_CXX_STANDARD 11)

include_directories(include /usr/local/Cellar/boost/1.69.0_2/include)

set(SOURCE_FILES
        include/ant/util/std_compat.hh
        include/ant/util/noncopyable.hh
        include/ant/util/noncopyable_function.hh
        include/ant/net/byteorder.hh
        include/ant/net/socket_defs.hh
        include/ant/net/ip.hh
        include/ant/byteorder.hh
        include/ant/deleter.hh
        include/ant/posix.hh
        include/ant/print.hh
        include/ant/sstring.hh
        include/ant/temporary_buffer.hh
        include/ant/unaligned.hh
        include/ant/lowres_timer_manager.hh
        include/ant/bitset_iter.hh
        include/ant/lowres_clock.hh
        include/ant/manual_clock.hh
        include/ant/cacheline.hh
        include/ant/timer_set.hh
        include/ant/timer.hh

        src/net/inet_address.cc
        src/net/ip.cc
        src/lowres_timer_manager.cc
        src/timer_manager.cc
        include/ant/util/defer.hh)


add_library(ant ${SOURCE_FILES})

add_subdirectory(demo)