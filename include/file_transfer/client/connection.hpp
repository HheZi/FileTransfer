#ifndef CLIENT_CONNECTION_HPP
#define CLIENT_CONNECTION_HPP

#include <array>
#include <boost/asio.hpp>
#include <boost/asio/impl/write.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include "file_transfer/common/encoder_decoder.hpp"

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Connection : public std::enable_shared_from_this<Connection> {
public:
  template<typename Res>
  using AsyncReader = std::function<void(const Res&)>;
  Connection(net::io_context &io_context, std::string host, std::string port)
      : io_context_(io_context), socket_(io_context_), host_(host), port_(port),
        is_connected_(false) {}

  void writeRawData(const char *data, const std::size_t size) {
    connectIfNotConnected();

    net::write(socket_, net::buffer(data, size));
  }

  template <typename Msg> void write(const Msg &msg) {
    connectIfNotConnected();

    std::vector<char> buf = EncoderDecoder::encode<Msg>(msg);

    net::write(socket_, net::buffer(buf));
  }

  template <typename Msg> Msg read() {
    connectIfNotConnected();

    return EncoderDecoder::decode<Msg>(socket_);
  }

  template <typename Msg, typename Res>
  void readAsync(const AsyncReader<Res> async_reader) {
    connectIfNotConnected();

    auto self = shared_from_this();
    socket_.async_read_some(
        net::buffer(buf_),
        [self, async_reader](const boost::system::error_code &err,
                             std::size_t bytes_transferred) {
          if (err)
            return;
          self->handleAsyncRead<Msg, Res>(async_reader);
        });
  }

private:
  net::io_context& io_context_;
  tcp::socket socket_;
  std::array<char, 1024> buf_;

  std::string host_;
  std::string port_;

  bool is_connected_;

  void connectIfNotConnected() {
    if (is_connected_)
      return;

    tcp::resolver resolver(io_context_);

    net::connect(socket_, resolver.resolve(host_, port_));

    is_connected_ = true;
  }

  template <typename Msg, typename Res> void handleAsyncRead(const AsyncReader<Res> async_reader) {
    Msg msg;

    try {
      msg = EncoderDecoder::decode<Msg>(buf_.data());
    } catch (...) {
      return;
    }

    Res res {msg};

    async_reader(res);
  }
};

#endif
