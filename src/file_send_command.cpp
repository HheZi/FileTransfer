#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <vector>
#include "connection.h"

constexpr std::size_t CHUNK_SIZE = 64 * 1024;

class FileSendCommand {
public:
  FileSendCommand(Connection& connection) : connection_(connection) {}

  void sendFile(std::string &filePath, std::uint64_t size)  {
    std::ifstream in(filePath, std::ios::binary);

    if (!in)
      throw std::runtime_error("Cannot open file");

    std::vector<char> buf(CHUNK_SIZE);

    std::uint64_t sent = 0;
    while (!in.eof() && sent < size) {
      std::size_t want = static_cast<std::size_t>(std::min(buf.size(), size - sent));
      in.read(buf.data(), buf.size());
      
      connection_.writeRawData(buf.data(), want);
      
      sent += want;
    }
  }

private:
  Connection& connection_;
};
