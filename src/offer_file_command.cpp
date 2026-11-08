#include <filesystem>
#include <stdexcept>
#include "connection.h"
#include "FileOffer.pb.h"

namespace fs = std::filesystem;

class OfferFileCommand {
public:
  OfferFileCommand(Connection& connection) : connection_(connection) {}

  bool offerFile(std::string &pathToFile) {
    if (!fs::exists(pathToFile)) {
      throw std::runtime_error("File does not exist");
    }

    sendFileOffer(pathToFile);

    FileAnswer fileAnswer = connection_.read<FileAnswer>();

    return fileAnswer.accepted();
  }

private:
  Connection& connection_;

  void sendFileOffer(std::string &pathToFile) {
    fs::path path{pathToFile};

    FileOffer fileOffer;

    fileOffer.set_name(path.filename());
    fileOffer.set_size(fs::file_size(path));

    connection_.write(fileOffer);
  }
};
