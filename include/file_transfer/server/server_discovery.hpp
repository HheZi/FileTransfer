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
  ServerDiscovery(net::io_context &io_context, std::string id,
                  std::uint32_t port, std::string name);

  void start();

  void stop();

private:
  static constexpr std::uint8_t TIMER_EXPIRY_TIME = 5;

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
