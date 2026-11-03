#include "connection.h"
#include "server.cpp"
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include "offer_file_command.cpp"
#include "file_send_command.cpp"
#include "connection.h"

void receiverMode(char *argv[]) {
  Server server (std::stoi(argv[2]), argv[3]);
  server.start();
}

void sendMode(char *argv[]) {
  Connection connection(argv[2], argv[3]);

  OfferFileCommand offerFile(connection);

  std::string filePath = argv[4];

  std::cout << "Offering file...\n";
  bool accepted = offerFile.offerFile(filePath);

  if (accepted)
    std::cout << "File offer has been accepted\n";
  else {
    std::cout << "File offer has been declined\n";
    return;
  }

  std::cout << "Start sending file\n";
  FileSendCommand fileSend(connection);

  std::filesystem::path file(filePath);
  fileSend.sendFile(filePath, std::filesystem::file_size(file));

  std::cout << "File has been sent\n";
}

int main(int argc, char *argv[]) {
  boost::asio::io_context io_context;
  std::string mode = argc > 1 ? argv[1] : "";

  try {
    if (mode == "recv") {
      if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <mode>, <port>, <working_directory>\n";
        return 1;
      }
      
      receiverMode(argv);
    } else if (mode == "send") {
      if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <mode>, <host>, <port>, <file_to_send>\n";
        return 1;
      }

      sendMode(argv);
    } else {
      std::cerr << "Wrong mode\n";
      return 1;
    }

  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
