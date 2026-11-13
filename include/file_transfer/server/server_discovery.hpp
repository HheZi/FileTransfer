#ifndef SERVER_SERVER_DISCOVERY_HPP
#define SERVER_SERVER_DISCOVERY_HPP

#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <cstdint>
#include <string>

namespace net = boost::asio;
using udp = boost::asio::ip::udp;

class ServerDiscovery {
public:
  static constexpr std::uint32_t BROADCAST_PORT = 45454;
  static constexpr std::uint32_t TIMER_EXPIRY_TIME = 5;

  ServerDiscovery(net::io_context &io_context, std::string id,
                  std::uint32_t port, std::string name);

  void start();

  void stop();

private:
  boost::asio::steady_timer timer_;

  std::string id_;
  std::string name_;
  std::uint32_t port_;

  udp::socket socket_;
  udp::endpoint endpoint_;

  void broadcast();

  void onStop();
};

#endif
