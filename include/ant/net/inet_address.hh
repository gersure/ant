/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*
 * Copyright (C) 2016 ScyllaDB.
 */

#pragma once

#include <iosfwd>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdexcept>
#include <vector>

#include "ant/sstring.hh"

namespace ant {
namespace net {

struct ipv4_address;
struct ipv6_address;

class unknown_host : public std::invalid_argument {
public:
    using invalid_argument::invalid_argument;
};

class inet_address {
public:
    enum class family : sa_family_t {
        INET = AF_INET, INET6 = AF_INET6
    };
private:
    family _in_family;

    union {
        ::in_addr _in;
        ::in6_addr _in6;
    };
public:

    inet_address();
    inet_address(family);
    inet_address(::in_addr i);
    inet_address(::in6_addr i);
    // NOTE: does _not_ resolve the address. Only parses
    // ipv4/ipv6 numerical address
    inet_address(const sstring&);
    inet_address(inet_address&&) = default;
    inet_address(const inet_address&) = default;

    inet_address(const ipv4_address&);
    inet_address(const ipv6_address&);

    // throws iff ipv6
    ipv4_address as_ipv4_address() const;
    ipv6_address as_ipv6_address() const;

    inet_address& operator=(const inet_address&) = default;
    bool operator==(const inet_address&) const;

    family in_family() const {
        return _in_family;
    }

    bool is_ipv6() const {
        return _in_family == family::INET6;
    }
    bool is_ipv4() const {
        return _in_family == family::INET;
    }

    size_t size() const;
    const void * data() const;

    operator ::in_addr() const;
    operator ::in6_addr() const;

    operator ipv6_address() const;

    sstring hostname() const;
    std::vector<sstring> aliases() const;

//    static inet_address find(const sstring&);
//    static inet_address find(const sstring&, family);
//    static std::vector<inet_address> find_all(const sstring&);
//    static fstd::vector<inet_address> find_all(const sstring&, family);
};

std::ostream& operator<<(std::ostream&, const inet_address&);
std::ostream& operator<<(std::ostream&, const inet_address::family&);

}
}

namespace std {
template<>
struct hash<ant::net::inet_address> {
    size_t operator()(const ant::net::inet_address&) const;
};
}
