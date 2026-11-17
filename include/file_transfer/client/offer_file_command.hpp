#ifndef CLIENT_OFFER_FILE_COMMAND_HPP
#define CLIENT_OFFER_FILE_COMMAND_HPP

#include "FileOffer.pb.h"
#include "connection.hpp"
#include <memory>

struct OfferResult {
  bool file_accepted;

  OfferResult() = default;
  OfferResult(const FileAnswer &answer) : file_accepted(answer.accepted()) {}
};

class OfferFileCommand {
public:
  OfferFileCommand(std::shared_ptr<Connection> connection);

  void offerFile(const std::string &pathToFile,
                 const Connection::AsyncReader<OfferResult> async_reader);

private:
  std::shared_ptr<Connection> connection_;
};

#endif
