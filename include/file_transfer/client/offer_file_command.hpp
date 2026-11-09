#ifndef CLIENT_OFFER_FILE_COMMAND_HPP
#define CLIENT_OFFER_FILE_COMMAND_HPP

#include "connection.hpp"

class OfferFileCommand {
public:
  OfferFileCommand(Connection& connection);

  bool offerFile(std::string &pathToFile);

private:
  Connection& connection_;

  void sendFileOffer(std::string &pathToFile);
};


#endif
