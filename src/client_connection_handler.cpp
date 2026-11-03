#include "proto/FileOffer.pb.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <memory>
#include <boost/asio.hpp>
#include <system_error>
#include "encoder_decoder.h"

class ClientConnectionHandler
    : public std::enable_shared_from_this<ClientConnectionHandler> {
public:
  using tcp = boost::asio::ip::tcp;
  using connection_ptr = std::shared_ptr<ClientConnectionHandler>;

  static connection_ptr create(boost::asio::io_context &io_context,
                               std::string &working_dir) {
    return connection_ptr(new ClientConnectionHandler(io_context, working_dir));
  }

  tcp::socket &socket() { return socket_; }

  void start() {
    try {
      std::cout << "Start accepting connection\n";

      FileOffer offer = EncoderDecoder::decode<FileOffer>(socket_);

      FileAnswer answer = decide(offer);

      auto buf = EncoderDecoder::encode(answer);

      boost::asio::write(socket_, boost::asio::buffer(buf));

      if (!answer.accepted()) {
        std::cout << "Close the connection\n";
        socket_.close();
        return;
      }

      acceptFile(offer.name(), offer.size());

      socket_.close();
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }

private:
  ClientConnectionHandler(boost::asio::io_context &io_context,
                          std::string &working_dir)
      : socket_(io_context), dir_(working_dir) {}
  tcp::socket socket_;
  std::filesystem::path dir_;

  static constexpr std::size_t CHUNK_SIZE = 64 * 1024;

  void acceptFile(const std::string &fileName, const std::uint64_t size) {
    std::filesystem::path file = dir_ / fileName;

    try {
      std::cout << "Start accepting file\n";

      std::ofstream out(file, std::ios::binary | std::ios::trunc);

      if (!out)
        throw std::runtime_error("Cannot create file");

      std::vector<char> buf(CHUNK_SIZE);
      std::uint64_t got = 0;
      while (got < size) {
        std::size_t want =
            static_cast<std::size_t>(std::min(buf.size(), size - got));
        std::size_t n = socket_.read_some(net::buffer(buf.data(), want));

        out.write(buf.data(), static_cast<std::streamsize>(n));
        got += n;
      }

      out.close();

      std::cout << "File received\n";
    } catch (...) {
      std::error_code ec;
      std::filesystem::remove(file, ec);
      throw;
    }
  }

  FileAnswer decide(FileOffer &offer) {
    std::cout << "Received file offer\n";
    std::cout << "File Name: " << offer.name() << std::endl;
    std::cout << "File size: " << offer.size() << std::endl << std::endl;

    std::string str_answer;
    std::cout << "Accept file(Y/N)?\n";
    std::getline(std::cin, str_answer);

    bool is_accepted = false;
    if (str_answer == "Y")
      is_accepted = true;

    FileAnswer answer;
    answer.set_accepted(is_accepted);

    return answer;
  }
};
