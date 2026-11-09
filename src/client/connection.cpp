#include <boost/asio.hpp>
#include <boost/asio/impl/write.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstddef>
#include <iostream>
#include "file_transfer/common/encoder_decoder.hpp"
#include "file_transfer/client/connection.hpp"

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

Connection::Connection(std::string host, std::string port)
    : io_context_(), socket_(io_context_), host_(host), port_(port),
      is_connected_(false) {}

void Connection::writeRawData(const char *data, const std::size_t size) {
  connectIfNotConnected();

  net::write(socket_, net::buffer(data, size));
}

template <typename Msg> void Connection::write(const Msg &msg) {
  connectIfNotConnected();

  std::vector<char> buf = EncoderDecoder::encode(msg);

  net::write(socket_, net::buffer(buf));
}

template <typename Msg> Msg Connection::read() {
  connectIfNotConnected();

  return EncoderDecoder::decode<Msg>(socket_);
}

void Connection::connectIfNotConnected() {
  if (is_connected_)
    return;

  tcp::resolver resolver(io_context_);

  std::cout << "Connecting to receiver...\n";
  net::connect(socket_, resolver.resolve(host_, port_));

  is_connected_ = true;

  std::cout << "Connected to receiver\n";
}
