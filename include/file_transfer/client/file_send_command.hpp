#ifndef CLIENT_FILE_SEND_COMMAND
#define CLIENT_FILE_SEND_COMMAND

#include "connection.hpp"
#include <generator>

class FileSendCommand {
public:
  FileSendCommand(std::shared_ptr<Connection> connection);

  std::generator<std::uint64_t> sendFile(const std::string &filePath);

private:
  std::shared_ptr<Connection> connection_;
};

#endif
