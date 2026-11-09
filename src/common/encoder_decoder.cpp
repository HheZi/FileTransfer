#include "file_transfer/common/encoder_decoder.hpp"
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/impl/read.hpp>
#include <boost/endian/conversion.hpp>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

template <typename Msg>
std::vector<char> EncoderDecoder::encode(const Msg &msg) {
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

template <typename Msg> Msg EncoderDecoder::decode(tcp::socket &socket) {
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

template <typename Msg> Msg EncoderDecoder::decode(const char *data) {
  Msg msg;

  std::uint32_t len = 0;
  std::memcpy(&len, data, sizeof len);
  len = boost::endian::big_to_native(len);

  std::string payload(data + sizeof len, len);

  if (!msg.ParseFromString(payload)) {
    throw std::runtime_error("Parse fail");
  }

  return msg;
}
