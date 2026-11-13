#include "FileOffer.pb.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <net/if.h>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <boost/asio.hpp>
#include <system_error>
#include "file_transfer/common/encoder_decoder.hpp"
#include "file_transfer/server/client_connection_handler.hpp"

namespace net = boost::asio;

ClientConnectionHandler::connection_ptr
ClientConnectionHandler::create(boost::asio::io_context &io_context,
                                std::string &working_dir, Decider &decider,
                                FileReceiveProgress &file_progress) {
  return connection_ptr(new ClientConnectionHandler(io_context, working_dir, decider, file_progress));
}

tcp::socket &ClientConnectionHandler::socket() { return socket_; }

void ClientConnectionHandler::start() {
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

ClientConnectionHandler::ClientConnectionHandler(
    boost::asio::io_context &io_context, std::string &working_dir,
    Decider &decider, FileReceiveProgress &file_progress)
    : socket_(io_context), dir_(working_dir), decider_(decider),
      file_progress_(file_progress) {}

void ClientConnectionHandler::acceptFile(const std::string &fileName,
                                         const std::uint64_t size) {
  std::filesystem::path file = dir_ / fileName;

  try {
    std::cout << "Start accepting file\n";

    std::ofstream out(file, std::ios::binary | std::ios::trunc);

    if (!out)
      throw std::runtime_error("Cannot create file");

    std::vector<char> buf(CHUNK_SIZE);
    std::uint64_t got = 0;
    file_progress_(size, got);
    while (got < size) {
      std::size_t want =
          static_cast<std::size_t>(std::min(buf.size(), size - got));
      std::size_t n = socket_.read_some(net::buffer(buf.data(), want));

      out.write(buf.data(), static_cast<std::streamsize>(n));
      got += n;

      file_progress_(size, got);
    }

    out.close();

    std::cout << "File received\n";
  } catch (...) {
    std::error_code ec;
    std::filesystem::remove(file, ec);
    throw;
  }
}

FileAnswer ClientConnectionHandler::decide(FileOffer &offer) {

  FileInfo info (offer);
  
  bool is_accepted = decider_(info);

  FileAnswer answer;
  answer.set_accepted(is_accepted);

  return answer;
}
