#ifndef CLIENT_DISCOVERY_HANDLER_HPP
#define CLIENT_DISCOVERY_HANDLER_HPP

#include <boost/asio.hpp>
#include <cstdint>
#include <functional>
#include <vector>
#include "Announce.pb.h"

namespace net = boost::asio;
using udp = boost::asio::ip::udp;

struct ClientDetails {
  std::string id;
  std::string name;
  std::string host;
  std::uint32_t port;
  bool is_closed;

  ClientDetails() = default;
  ClientDetails(const Announce &announce, const udp::endpoint &endpoint)
    : id(announce.id()), name(announce.name()),
      host(endpoint.address().to_string()),
      port(announce.port()), is_closed(announce.isclosed()) {}
};

class DiscoveryHandler {
public:
  using Observer = std::function<void(const ClientDetails &)>;

  DiscoveryHandler(net::io_context &io, std::string id);

  void startReceive();

  void addObserver(Observer observer);

private:
  static constexpr std::uint32_t BROADCAST_PORT = 45454;
  
  udp::socket socket_;

  std::string id_;
  std::array<char, 1024> rx_;
  udp::endpoint sender_;

  std::vector<Observer> observers;

  void handle();
};

#endif
