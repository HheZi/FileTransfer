#ifndef CLIENT_FILE_SEND_COMMAND
#define CLIENT_FILE_SEND_COMMAND

#include "connection.hpp"
#include <cstddef>
#include <generator>

class FileSendCommand {
public:
  FileSendCommand(Connection &connection);

  std::generator<std::uint64_t> sendFile(const std::string &filePath);

private:
  static constexpr std::size_t CHUNK_SIZE = 64 * 1024;

  Connection &connection_;
};

#endif
