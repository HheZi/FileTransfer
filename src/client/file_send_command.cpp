#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <vector>
#include "file_transfer/common/constants.hpp"
#include "file_transfer/client/file_send_command.hpp"

namespace fs = std::filesystem;

FileSendCommand::FileSendCommand(std::shared_ptr<Connection> connection,
                                 const std::string file_path)
    : connection_(connection), file_path_(file_path) {}
FileSendCommand::~FileSendCommand() {
  if (in_) {
    in_->close();
  }
}

std::generator<std::uint64_t> FileSendCommand::sendFile() {
  std::uint64_t total_size = fs::file_size(file_path_);

  in_ = std::make_shared<std::ifstream>(file_path_, std::ios::binary);

  if (!in_)
    throw std::runtime_error("Cannot open file");

  std::vector<char> buf(CHUNK_SIZE);

  std::uint64_t sent = 0;

  co_yield sent;

  try {
    while (sent < total_size && in_) {
      std::streamsize want =
          static_cast<std::streamsize>(std::min(buf.size(), total_size - sent));

      in_->read(buf.data(), want);

      std::streamsize bytesRead = in_->gcount();

      if (bytesRead == 0)
        break;

      connection_->writeRawData(buf.data(),
                                static_cast<std::size_t>(bytesRead));

      sent += bytesRead;

      co_yield sent;
    }

  } catch (...) {
    cancel();
    throw;
  }

  in_->close();
}

void FileSendCommand::cancel() {
  connection_->close();

  if (in_) {
    in_->close();
  }
}
