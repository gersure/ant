//
// Created by zhengcf on 2019-06-26.
//

#include "ant/net/ip.hh"
#include "ant/print.hh"

namespace ant {

net::ipv4_address::ipv4_address(const std::string &addr) {
    boost::system::error_code ec;
    auto ipv4 = boost::asio::ip::address_v4::from_string(addr, ec);
    if (ec) {
        throw std::runtime_error(
                format("Wrong format for IPv4 address {}. Please ensure it's in dotted-decimal format", addr));
    }
    ip = static_cast<uint32_t>(std::move(ipv4).to_ulong());
}


} // namespace ant