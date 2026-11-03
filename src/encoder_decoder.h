#pragma once

#include <array>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/impl/read.hpp>
#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class EncoderDecoder {
public:
  template <typename Msg>
  static std::vector<char> encode(const Msg &msg) {
    std::string data;
    if (!msg.SerializeToString(&data)) {
      throw std::runtime_error("Serialize fail");
    }

    std::uint32_t len =
        boost::endian::native_to_big(static_cast<std::uint32_t>(data.size()));

    std::vector<char> out(sizeof len + data.size());
    std::memcpy(out.data(), &len, sizeof len);
    std::memcpy(out.data() + sizeof len, data.data(), data.size());

    return out;
  }

  template <typename Msg>
  static Msg decode(tcp::socket &socket) {
    Msg msg;
    std::uint32_t len = 0;

    net::read(socket, net::buffer(&len, sizeof len));
    len = boost::endian::big_to_native(len);

    std::string payload(len, '\0');
    net::read(socket, net::buffer(payload));

    if (!msg.ParseFromString(payload)) {
      throw std::runtime_error("Parse fail");
    }
    
    return msg;
  }
};
