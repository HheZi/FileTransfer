#include "file_transfer/server/file_receiver.hpp"
#include <boost/system/detail/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <fstream>
#include <generator>
#include <memory>
#include "file_transfer/common/constants.hpp"

using tcp = boost::asio::ip::tcp;
namespace fs = std::filesystem;
namespace net = boost::asio;

std::shared_ptr<FileReceiver>
FileReceiver::create(tcp::socket &socket, const fs::path dir,
                     const std::string file_name,
                     const std::uint64_t file_size) {
  return std::shared_ptr<FileReceiver>(new FileReceiver(socket, dir, file_name, file_size));
}

FileReceiver::FileReceiver(boost::asio::ip::tcp::socket &socket,
                           const std::filesystem::path dir,
                           const std::string file_name,
                           const std::uint64_t file_size)
    : socket_(std::move(socket)), dir_(dir), file_name(file_name),
      file_size(file_size), is_stoped_(false) {}

FileReceiver::~FileReceiver() {
  closeConnection();
  if (out_)
    out_->close();
}

std::generator<std::uint64_t> FileReceiver::receiveFile() {
  fs::path file = dir_ / file_name;

  try {
    out_ = std::make_shared<std::ofstream>(file, std::ios::binary | std::ios::trunc);

    if (!out_)
      throw std::runtime_error("Cannot create file");

    std::vector<char> buf(CHUNK_SIZE);
    std::uint64_t got = 0;
    co_yield got;

    while (!is_stoped_ && got < file_size) {
      std::size_t want =
          static_cast<std::size_t>(std::min(buf.size(), file_size - got));
      std::size_t n = socket_.read_some(net::buffer(buf.data(), want));

      out_->write(buf.data(), static_cast<std::streamsize>(n));
      got += n;

      co_yield got;
    }

    closeConnection();

    out_->close();
  } catch (...) {
    stop();
    throw;
  }
}


void FileReceiver::stop() {
  closeConnection();
  deleteFile();
}

void FileReceiver::closeConnection() {
  if (socket_.is_open()) {
    socket_.shutdown(tcp::socket::shutdown_both);
    socket_.close();
  }
  is_stoped_ = true;
}

void FileReceiver::deleteFile() {
  if (out_)
    out_->close();

  fs::path file = dir_ / file_name;
  std::error_code ec;
  std::filesystem::remove(file, ec);
}
