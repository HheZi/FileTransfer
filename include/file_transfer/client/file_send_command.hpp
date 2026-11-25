#ifndef CLIENT_FILE_SEND_COMMAND
#define CLIENT_FILE_SEND_COMMAND

#include "connection.hpp"
#include <fstream>
#include <generator>
#include <memory>

class FileSendCommand {
public:
  FileSendCommand(std::shared_ptr<Connection> connection,
                  const std::string filePath);
  ~FileSendCommand();

  std::generator<std::uint64_t> sendFile();

  void cancel();

private:
  const std::string file_path_;

  std::shared_ptr<Connection> connection_;

  std::shared_ptr<std::ifstream> in_;
};

#endif
