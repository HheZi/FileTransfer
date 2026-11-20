#ifndef CLIENT_OFFER_FILE_COMMAND_HPP
#define CLIENT_OFFER_FILE_COMMAND_HPP

#include "file_transfer/client/offer_result.hpp"
#include "connection.hpp"
#include <memory>

class OfferFileCommand {
public:
  OfferFileCommand(std::shared_ptr<Connection> connection);

  void offerFile(const std::string &pathToFile,
                 const Connection::AsyncReader<OfferResult> async_reader);

private:
  std::shared_ptr<Connection> connection_;
};

#endif
