#pragma once

#include <boost/asio/ip/address_v4.hpp>
#include <arpa/inet.h>
#include <cstdint>
#include <array>
#include <string>
#include "ant/net/byteorder.hh"
#include "ant/byteorder.hh"

namespace ant {

struct ipv6_addr;
struct ipv4_addr;

namespace net {

struct ipv4_address;

struct ipv4_address {
    ipv4_address() : ip(0) {}

    explicit ipv4_address(uint32_t ip) : ip(ip) {}

    explicit ipv4_address(const std::string &addr);

    ipv4_address(ipv4_addr addr);

    packed<uint32_t> ip;

    friend bool operator==(ipv4_address x, ipv4_address y) {
        return x.ip == y.ip;
    }

    friend bool operator!=(ipv4_address x, ipv4_address y) {
        return x.ip != y.ip;
    }

    static ipv4_address read(const char *p) {
        ipv4_address ia;
        ia.ip = read_be<uint32_t>(p);
        return ia;
    }

    static ipv4_address consume(const char *&p) {
        auto ia = read(p);
        p += 4;
        return ia;
    }

    void write(char *p) const {
        write_be<uint32_t>(p, ip);
    }

    void produce(char *&p) const {
        produce_be<uint32_t>(p, ip);
    }

    static constexpr size_t size() {
        return 4;
    }
} __attribute__((packed));

static inline bool is_unspecified(ipv4_address addr) { return addr.ip == 0; }

std::ostream &operator<<(std::ostream &os, const ipv4_address &a);

// IPv6
struct ipv6_address {
    using ipv6_bytes = std::array<uint8_t, 16>;

    static_assert(alignof(ipv6_bytes) == 1, "ipv6_bytes should be byte-aligned");
    static_assert(sizeof(ipv6_bytes) == 16, "ipv6_bytes should be 16 bytes");

    ipv6_address();

    explicit ipv6_address(const ::in6_addr &);

    explicit ipv6_address(const ipv6_bytes &);

    explicit ipv6_address(const std::string &);

    ipv6_address(const ipv6_addr &addr);

    // No need to use packed - we only store
    // as byte array. If we want to read as
    // uints or whatnot, we must copy
    ipv6_bytes ip;

    bool operator==(const ipv6_address &y) const {
        return bytes() == y.bytes();
    }

    bool operator!=(const ipv6_address &y) const {
        return !(*this == y);
    }

    const ipv6_bytes &bytes() const {
        return ip;
    }

    bool is_unspecified() const;

    static ipv6_address read(const char *);

    static ipv6_address consume(const char *&p);

    void write(char *p) const;

    void produce(char *&p) const;

    static constexpr size_t size() {
        return sizeof(ipv6_bytes);
    }
} __attribute__((packed));

std::ostream &operator<<(std::ostream &, const ipv6_address &);

}

}

namespace std {

template<>
struct hash<ant::net::ipv4_address> {
    size_t operator()(ant::net::ipv4_address a) const { return a.ip; }
};

template<>
struct hash<ant::net::ipv6_address> {
    size_t operator()(const ant::net::ipv6_address &) const;
};

}
