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

#include <ostream>
#include <arpa/inet.h>
#include <boost/functional/hash.hpp>
#include "ant/net/inet_address.hh"
#include "ant/net/socket_defs.hh"
#include "ant/net/ip.hh"
#include "ant/print.hh"

ant::net::inet_address::inet_address()
        : inet_address(::in6_addr{})
{}

ant::net::inet_address::inet_address(family f)
        : _in_family(f)
{
    memset(&_in6, 0, sizeof(_in6));
}

ant::net::inet_address::inet_address(::in_addr i)
        : _in_family(family::INET), _in(i) {
}

ant::net::inet_address::inet_address(::in6_addr i)
        : _in_family(family::INET6), _in6(i) {
}

ant::net::inet_address::inet_address(const sstring& addr)
        : inet_address([&addr] {
    inet_address in;
    if (::inet_pton(AF_INET, addr.c_str(), &in._in)) {
        in._in_family = family::INET;
        return in;
    }
    if (::inet_pton(AF_INET6, addr.c_str(), &in._in6)) {
        in._in_family = family::INET6;
        return in;
    }
    throw std::invalid_argument(addr);
}())
{}

ant::net::inet_address::inet_address(const ipv4_address& in)
        : inet_address(::in_addr{hton(in.ip)})
{}

ant::net::inet_address::inet_address(const ipv6_address& in)
        : inet_address([&] {
    ::in6_addr tmp;
    std::copy(in.bytes().begin(), in.bytes().end(), tmp.s6_addr);
    return tmp;
}())
{}

ant::net::ipv4_address ant::net::inet_address::as_ipv4_address() const {
    in_addr in = *this;
    return ipv4_address(ntoh(in.s_addr));
}

ant::net::ipv6_address ant::net::inet_address::as_ipv6_address() const {
    in6_addr in6 = *this;
    return ipv6_address{in6};
}

bool ant::net::inet_address::operator==(const inet_address& o) const {
    if (o._in_family != _in_family) {
        return false;
    }
    switch (_in_family) {
        case family::INET:
            return _in.s_addr == o._in.s_addr;
        case family::INET6:
            return std::equal(std::begin(_in6.s6_addr), std::end(_in6.s6_addr),
                              std::begin(o._in6.s6_addr));
        default:
            return false;
    }
}

ant::net::inet_address::operator ::in_addr() const {
    if (_in_family != family::INET) {
        if (IN6_IS_ADDR_V4MAPPED(&_in6)) {
            ::in_addr in;
            in.s_addr = _in6.s6_addr32[3];
            return in;
        }
        throw std::invalid_argument("Not an IPv4 address");
    }
    return _in;
}

ant::net::inet_address::operator ::in6_addr() const {
    if (_in_family == family::INET) {
        in6_addr in6 = IN6ADDR_ANY_INIT;
        in6.s6_addr32[2] = ::htonl(0xffff);
        in6.s6_addr32[3] = _in.s_addr;
        return in6;
    }
    return _in6;
}

ant::net::inet_address::operator ant::net::ipv6_address() const {
    return as_ipv6_address();
}

size_t ant::net::inet_address::size() const {
    switch (_in_family) {
        case family::INET:
            return sizeof(::in_addr);
        case family::INET6:
            return sizeof(::in6_addr);
        default:
            return 0;
    }
}

const void * ant::net::inet_address::data() const {
    return &_in;
}

ant::net::ipv6_address::ipv6_address(const ::in6_addr& in) {
    std::copy(std::begin(in.s6_addr), std::end(in.s6_addr), ip.begin());
}

ant::net::ipv6_address::ipv6_address(const ipv6_bytes& in)
        : ip(in)
{}

ant::net::ipv6_address::ipv6_address(const ipv6_addr& addr)
        : ipv6_address(addr.ip)
{}

ant::net::ipv6_address::ipv6_address()
        : ipv6_address(::in6addr_any)
{}

ant::net::ipv6_address::ipv6_address(const std::string& addr) {
    if (!::inet_pton(AF_INET6, addr.c_str(), ip.data())) {
        throw std::runtime_error(format("Wrong format for IPv6 address {}. Please ensure it's in colon-hex format",
                                        addr));
    }
}

ant::net::ipv6_address ant::net::ipv6_address::read(const char* s) {
    auto* b = reinterpret_cast<const uint8_t *>(s);
    ipv6_address in;
    std::copy(b, b + ipv6_address::size(), in.ip.begin());
    return in;
}

ant::net::ipv6_address ant::net::ipv6_address::consume(const char*& p) {
    auto res = read(p);
    p += size();
    return res;
}

void ant::net::ipv6_address::write(char* p) const {
    std::copy(ip.begin(), ip.end(), p);
}

void ant::net::ipv6_address::produce(char*& p) const {
    write(p);
    p += size();
}

bool ant::net::ipv6_address::is_unspecified() const {
    return std::all_of(ip.begin(), ip.end(), [](uint8_t b) { return b == 0; });
}

std::ostream& ant::net::operator<<(std::ostream& os, const ipv4_address& a) {
    auto ip = a.ip;
    return fmt_print(os, "{:d}.{:d}.{:d}.{:d}",
                     (ip >> 24) & 0xff,
                     (ip >> 16) & 0xff,
                     (ip >> 8) & 0xff,
                     (ip >> 0) & 0xff);
}

std::ostream& ant::net::operator<<(std::ostream& os, const ipv6_address& a) {
    char buffer[64];
    return os << ::inet_ntop(AF_INET6, a.ip.data(), buffer, sizeof(buffer));
}


ant::net::ipv4_address::ipv4_address(ant::ipv4_addr addr) {
    ip = addr.ip;
}
ant::ipv6_addr::ipv6_addr(const ipv6_bytes& b, uint16_t p)
        : ip(b), port(p)
{}

ant::ipv6_addr::ipv6_addr(uint16_t p)
        : ipv6_addr(net::inet_address(), p)
{}

ant::ipv6_addr::ipv6_addr(const ::in6_addr& in6, uint16_t p)
        : ipv6_addr(net::ipv6_address(in6).bytes(), p)
{}

ant::ipv6_addr::ipv6_addr(const std::string& s)
        : ipv6_addr([&] {
    auto lc = s.find_last_of(']');
    auto cp = s.find_first_of(':', lc);
    auto port = cp != std::string::npos ? std::stoul(s.substr(cp + 1)) : 0;
    auto ss = lc != std::string::npos ? s.substr(1, lc - 1) : s;
    return ipv6_addr(net::ipv6_address(ss).bytes(), uint16_t(port));
}())
{}

ant::ipv6_addr::ipv6_addr(const std::string& s, uint16_t p)
        : ipv6_addr(net::ipv6_address(s).bytes(), p)
{}

ant::ipv6_addr::ipv6_addr(const net::inet_address& i, uint16_t p)
        : ipv6_addr(i.as_ipv6_address().bytes(), p)
{}

ant::ipv6_addr::ipv6_addr(const ::sockaddr_in6& s)
        : ipv6_addr(s.sin6_addr, net::ntoh(s.sin6_port))
{}

ant::ipv6_addr::ipv6_addr(const socket_address& s)
        : ipv6_addr(s.as_posix_sockaddr_in6())
{}

bool ant::ipv6_addr::is_ip_unspecified() const {
    return std::all_of(ip.begin(), ip.end(), [](uint8_t b) { return b == 0; });
}

ant::socket_address::socket_address(const net::inet_address& a, uint16_t p)
        : socket_address(a.is_ipv6() ? socket_address(ipv6_addr(a, p)) : socket_address(ipv4_addr(a, p)))
{}

ant::net::inet_address ant::socket_address::addr() const {
    switch (as_posix_sockaddr().sa_family) {
        case AF_INET:
            return net::inet_address(as_posix_sockaddr_in().sin_addr);
        case AF_INET6:
            return net::inet_address(as_posix_sockaddr_in6().sin6_addr);
        default:
            return net::inet_address();
    }
}

::in_port_t ant::socket_address::port() const {
    return net::ntoh(u.in.sin_port);
}

bool ant::socket_address::is_wildcard() const {
    if (u.sa.sa_family == AF_INET) {
        ipv4_addr addr(*this);
        return addr.is_ip_unspecified() && addr.is_port_unspecified();
    } else {
        ipv6_addr addr(*this);
        return addr.is_ip_unspecified() && addr.is_port_unspecified();
    }
}

std::ostream& ant::net::operator<<(std::ostream& os, const inet_address& addr) {
    char buffer[64];
    return os << inet_ntop(int(addr.in_family()), addr.data(), buffer, sizeof(buffer));
}

std::ostream& ant::net::operator<<(std::ostream& os, const inet_address::family& f) {
    switch (f) {
        case inet_address::family::INET:
            os << "INET";
            break;
        case inet_address::family::INET6:
            os << "INET6";
            break;
        default:
            break;
    }
    return os;
}

std::ostream& ant::operator<<(std::ostream& os, const socket_address& a) {
    auto addr = a.addr();
    // CMH. maybe skip brackets for ipv4-mapped
    auto bracket = addr.in_family() == ant::net::inet_address::family::INET6;

    if (bracket) {
        os << '[';
    }
    os << addr;
    if (bracket) {
        os << ']';
    }

    return os << ':' << ntohs(a.u.in.sin_port);
}

std::ostream& ant::operator<<(std::ostream& os, const ipv4_addr& a) {
    return os << ant::socket_address(a);
}

std::ostream& ant::operator<<(std::ostream& os, const ipv6_addr& a) {
    return os << ant::socket_address(a);
}

size_t std::hash<ant::net::inet_address>::operator()(const ant::net::inet_address& a) const {
    switch (a.in_family()) {
        case ant::net::inet_address::family::INET:
            return std::hash<ant::net::ipv4_address>()(a.as_ipv4_address());
        case ant::net::inet_address::family::INET6:
            return std::hash<ant::net::ipv6_address>()(a.as_ipv6_address());
        default:
            return 0;
    }
}

size_t std::hash<ant::socket_address>::operator()(const ant::socket_address& a) const {
    auto h = std::hash<ant::net::inet_address>()(a.addr());
    boost::hash_combine(h, a.as_posix_sockaddr_in().sin_port);
    return h;
}

size_t std::hash<ant::net::ipv6_address>::operator()(const ant::net::ipv6_address& a) const {
    return boost::hash_range(a.ip.begin(), a.ip.end());
}

size_t std::hash<ant::ipv4_addr>::operator()(const ant::ipv4_addr& x) const {
    size_t h = x.ip;
    boost::hash_combine(h, x.port);
    return h;
}
