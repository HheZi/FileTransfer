#ifndef CLIENT_CLIENT_DETAILS_HPP
#define CLIENT_CLIENT_DETAILS_HPP

#include <boost/asio.hpp>
#include <string>
#include "Announce.pb.h"

struct ClientDetails {
  std::string id;
  std::string name;
  std::string host;
  std::uint32_t port;
  bool is_closed;

  ClientDetails() = default;
  ClientDetails(const Announce &announce, const boost::asio::ip::udp::endpoint &endpoint)
    : id(announce.id()), name(announce.name()),
      host(endpoint.address().to_string()),
      port(announce.port()), is_closed(announce.isclosed()) {}
};

#endif
