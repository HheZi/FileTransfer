#include <filesystem>
#include <memory>
#include <stdexcept>
#include "FileOffer.pb.h"
#include "file_transfer/client/connection.hpp"
#include "file_transfer/client/offer_file_command.hpp"

namespace fs = std::filesystem;

OfferFileCommand::OfferFileCommand(std::shared_ptr<Connection> connection)
    : connection_(connection) {}

void OfferFileCommand::offerFile(
    const std::string &pathToFile,
    const Connection::AsyncReader<OfferResult> async_reader) {
  if (!fs::exists(pathToFile)) {
    throw std::runtime_error("File does not exist");
  }

  fs::path path{pathToFile};

  FileOffer fileOffer;

  fileOffer.set_name(path.filename());
  fileOffer.set_size(fs::file_size(path));

  connection_->write(fileOffer);

  connection_->readAsync<FileAnswer, OfferResult>(async_reader);
}
