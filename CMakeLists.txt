cmake_minimum_required(VERSION 3.0.2)
project(ant)

set(CMAKE_CXX_STANDARD 11)

include_directories(include /usr/local/Cellar/boost/1.69.0_2/include)

set(SOURCE_FILES
        include/ant/net/byteorder.hh
        include/ant/net/socket_defs.hh
        include/ant/net/ip.hh
        include/ant/util/std-compat.hh
        include/ant/byteorder.hh
        include/ant/deleter.hh
        include/ant/posix.hh
        include/ant/print.hh
        include/ant/sstring.hh
        include/ant/temporary_buffer.hh
        include/ant/unaligned.hh
        include/ant/timer_manager.hh

        src/net/inet_address.cc
        src/net/ip.cc
        src/timer_manager.cc
        )


add_library(ant ${SOURCE_FILES})

add_subdirectory(demo)