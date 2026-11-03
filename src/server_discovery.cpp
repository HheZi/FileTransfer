#include "proto/Announce.pb.h"
#include <boost/asio.hpp>
#include <boost/asio/detail/chrono.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <cstdint>
#include <string>
#include "encoder_decoder.h"

namespace net = boost::asio;
using udp = boost::asio::ip::udp;

class ServerDiscovery {
public:
  static constexpr std::uint32_t BROADCAST_PORT = 45454;
  static constexpr std::uint32_t TIMER_EXPIRY_TIME = 5;

  ServerDiscovery(net::io_context &io_context, std::string id,
                  std::uint32_t port, std::string name)
      : timer_(io_context, boost::asio::chrono::seconds(TIMER_EXPIRY_TIME)),
        id_(id), port_(port), name_(name), socket_(io_context),
        endpoint_(boost::asio::ip::address_v4::broadcast(), BROADCAST_PORT) {};

  void start() {
    socket_.open(endpoint_.protocol());
    socket_.set_option(udp::socket::reuse_address(true));
    socket_.set_option(udp::socket::broadcast(true));
    socket_.bind(udp::endpoint(udp::v4(), BROADCAST_PORT));

    broadcast();
  }

private:
  boost::asio::steady_timer timer_;

  std::string id_;
  std::string name_;
  std::uint32_t port_;

  udp::socket socket_;
  udp::endpoint endpoint_;

  void broadcast() {
    Announce announce;
    announce.set_id(id_);
    announce.set_port(port_);
    announce.set_name(name_);

    std::vector<char> buf = EncoderDecoder::encode(announce);

    socket_.send_to(boost::asio::buffer(buf), endpoint_);

    timer_.expires_at(timer_.expiry() +
                      boost::asio::chrono::seconds(TIMER_EXPIRY_TIME));

    timer_.async_wait([this](const boost::system::error_code &ec) {
      if (!ec)
        broadcast();
    });
  }
};
