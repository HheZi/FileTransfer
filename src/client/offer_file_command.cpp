#include <filesystem>
#include <stdexcept>
#include "FileOffer.pb.h"
#include "file_transfer/client/offer_file_command.hpp"

namespace fs = std::filesystem;

OfferFileCommand::OfferFileCommand(Connection &connection)
    : connection_(connection) {}

bool OfferFileCommand::offerFile(std::string &pathToFile) {
  if (!fs::exists(pathToFile)) {
    throw std::runtime_error("File does not exist");
  }

  sendFileOffer(pathToFile);

  FileAnswer fileAnswer = connection_.read<FileAnswer>();

  return fileAnswer.accepted();
}

void OfferFileCommand::sendFileOffer(std::string &pathToFile) {
  fs::path path{pathToFile};

  FileOffer fileOffer;

  fileOffer.set_name(path.filename());
  fileOffer.set_size(fs::file_size(path));

  connection_.write(fileOffer);
}
