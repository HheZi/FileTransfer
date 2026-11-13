#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <vector>
#include "file_transfer/client/file_send_command.hpp"

namespace fs = std::filesystem;

FileSendCommand::FileSendCommand(Connection &connection)
    : connection_(connection) {}

std::generator<std::uint64_t> FileSendCommand::sendFile(const std::string &filePath) {
  std::uint64_t total_size = fs::file_size(filePath);

  std::ifstream in(filePath, std::ios::binary);

  if (!in)
    throw std::runtime_error("Cannot open file");

  std::vector<char> buf(CHUNK_SIZE);

  std::uint64_t sent = 0;

  co_yield sent;
  
  while (sent < total_size && in) {
    std::streamsize want =
        static_cast<std::streamsize>(std::min(buf.size(), total_size - sent));
    
    in.read(buf.data(), want);

    std::streamsize bytesRead = in.gcount();

    if (bytesRead == 0)
      break;

    connection_.writeRawData(buf.data(), static_cast<std::size_t>(bytesRead));

    sent += bytesRead;

    co_yield sent;
  }
}
