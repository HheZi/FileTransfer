#include <boost/system/detail/error_code.hpp>
#include "file_transfer/common/encoder_decoder.hpp"
#include "file_transfer/client/discovery_handler.hpp"

namespace net = boost::asio;
using udp = boost::asio::ip::udp;

DiscoveryHandler::DiscoveryHandler(net::io_context &io, std::string id)
    : socket_(io), id_(id) {
  udp::endpoint ep(udp::v4(), BROADCAST_PORT);
  socket_.open(ep.protocol());
  socket_.set_option(net::socket_base::reuse_address(true));
  socket_.set_option(net::socket_base::broadcast(true));
  socket_.bind(ep);
}

void DiscoveryHandler::startReceive() {
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

void DiscoveryHandler::addObserver(Observer observer) {
  observers.push_back(observer);
}

void DiscoveryHandler::handle() {
  Announce announce;

  try {
    announce = EncoderDecoder::decode<Announce>(rx_.data());
  } catch (...) {
    return;
  }

  if (announce.id() == id_) {
    return;
  }

  ClientDetails details{announce, sender_};

  for (Observer &observer : observers) {
    observer(details);
  }
}
