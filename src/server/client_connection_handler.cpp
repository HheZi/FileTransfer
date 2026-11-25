#include "FileOffer.pb.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <memory>
#include <net/if.h>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <boost/asio.hpp>
#include "file_transfer/common/encoder_decoder.hpp"
#include "file_transfer/server/file_receiver.hpp"
#include "file_transfer/server/client_connection_handler.hpp"

ClientConnectionHandler::connection_ptr
ClientConnectionHandler::create(boost::asio::io_context &io_context,
                                std::string &working_dir, Decider &decider,
                                FileReceiverHandler &file_receiver_handler) {
  return connection_ptr(new ClientConnectionHandler(
      io_context, working_dir, decider, file_receiver_handler));
}

tcp::socket &ClientConnectionHandler::socket() { return socket_; }

void ClientConnectionHandler::start() {
  try {

    FileOffer offer = EncoderDecoder::decode<FileOffer>(socket_);

    FileAnswer answer = decide(offer);

    auto buf = EncoderDecoder::encode(answer);

    boost::asio::write(socket_, boost::asio::buffer(buf));

    if (!answer.accepted()) {
      socket_.close();
      return;
    }

    std::shared_ptr<FileReceiver> file_receiver = FileReceiver::create(
        socket_, dir_, offer.name(), offer.size());

    file_receiver_hanlder_(file_receiver);
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}

ClientConnectionHandler::ClientConnectionHandler(
    boost::asio::io_context &io_context, std::string &working_dir,
    Decider &decider, FileReceiverHandler &file_receiver_handler)
    : socket_(io_context), dir_(working_dir), decider_(decider),
      file_receiver_hanlder_(file_receiver_handler) {}

FileAnswer ClientConnectionHandler::decide(FileOffer &offer) {

  FileInfo info (offer);

  bool is_accepted = decider_(info);

  FileAnswer answer;
  answer.set_accepted(is_accepted);

  return answer;
}
