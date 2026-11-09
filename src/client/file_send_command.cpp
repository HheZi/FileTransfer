#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <vector>
#include "file_transfer/client/file_send_command.hpp"

FileSendCommand::FileSendCommand(Connection &connection)
    : connection_(connection) {}

void FileSendCommand::sendFile(std::string &filePath, std::uint64_t size) {
  std::ifstream in(filePath, std::ios::binary);

  if (!in)
    throw std::runtime_error("Cannot open file");

  std::vector<char> buf(CHUNK_SIZE);

  std::uint64_t sent = 0;
  while (!in.eof() && sent < size) {
    std::size_t want =
        static_cast<std::size_t>(std::min(buf.size(), size - sent));
    in.read(buf.data(), buf.size());

    connection_.writeRawData(buf.data(), want);

    sent += want;
  }
}
