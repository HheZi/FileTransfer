#ifndef CLIENT_FILE_SEND_COMMAND
#define CLIENT_FILE_SEND_COMMAND

#include "connection.hpp"
#include <cstddef>
#include <cstdint>

class FileSendCommand {
public:
  FileSendCommand(Connection &connection);

  void sendFile(std::string &filePath, std::uint64_t size);

private:
  static constexpr std::size_t CHUNK_SIZE = 64 * 1024;

  Connection &connection_;
};

#endif
