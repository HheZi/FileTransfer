#include <boost/asio.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>
#include "proto/Announce.pb.h"
#include "encoder_decoder.h"

namespace net = boost::asio;
using udp = boost::asio::ip::udp;

struct ClientDetails {
  std::string id;
  std::string name;
  std::string host;
  std::uint32_t port;

  ClientDetails() = default;
  ClientDetails(const Announce &announce, const udp::endpoint &endpoint)
    : id(announce.id()), name(announce.name()),
      host(endpoint.address().to_string()),
      port(announce.port()) {}
};

class DiscoveryHandler {
public:
  using Observer = std::function<void(const ClientDetails &)>;

  static constexpr std::uint32_t BROADCAST_PORT = 45454;

  DiscoveryHandler(net::io_context &io, std::string id)
      : socket_(io), id_(id) {
    udp::endpoint ep(udp::v4(), BROADCAST_PORT);
    socket_.open(ep.protocol());
    socket_.set_option(net::socket_base::reuse_address(true));
    socket_.set_option(net::socket_base::broadcast(true));
    socket_.bind(ep);
  }

  void startReceive() {
    socket_.async_receive_from(
        net::buffer(rx_), sender_,
        [this](boost::system::error_code ec, std::size_t bytes) {
          if (ec == net::error::operation_aborted)
            return;

          if (!ec)
            handle();

          startReceive();
        });
  }

  void addObserver(Observer observer) { observers.push_back(observer); }

private:
  udp::socket socket_;

  std::string id_;
  std::array<char, 1024> rx_;
  udp::endpoint sender_;

  std::vector<Observer> observers;

  void handle() {
    Announce announce;

    try {
      announce = EncoderDecoder::decode<Announce>(rx_.data());
    } catch (...) {
      return;
    }

    if (announce.id() == id_) {
      return;
    }

    ClientDetails details {announce, sender_};

    for (Observer &observer : observers) {
      observer(details);
    }
  }
};
