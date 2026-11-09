#include "file_transfer/client/connection.hpp"
#include "file_transfer/server/server.hpp"
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <exception>
#include <iostream>
#include <filesystem>
#include <string>
#include "file_transfer/client/offer_file_command.hpp"
#include "file_transfer/client/file_send_command.hpp"
#include "file_transfer/client/discovery_handler.hpp"
#include "file_transfer/common/id_generator.hpp"

void receiverMode(int argc, char *argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0]
              << " <mode>, <port>, <working_directory>, <name>\n";
    exit(1);
  }

  Server server(std::stoi(argv[2]), argv[3], IdGenerator::generate(), argv[4]);
  server.start();
}

void sendMode(int argc, char *argv[]) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0]
              << " <mode>, <host>, <port>, <file_to_send>\n";
    exit(1);
  }

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

void discoveryMode(int argc, char *argv[]) {
  boost::asio::io_context context;

  DiscoveryHandler handler(context, IdGenerator::generate());

  auto func = [](const ClientDetails &details) {
    std::cout << "New ClientDetails\n";
    std::cout << "Id: " << details.id << std::endl;
    std::cout << "Name: " << details.name << std::endl;
    std::cout << "Host: " << details.host << std::endl;
    std::cout << "Port: " << details.port << std::endl;

    std::cout << std::endl;
  };

  handler.addObserver(func);
  handler.startReceive();
  
  context.run();
}

int main(int argc, char *argv[]) {
  boost::asio::io_context io_context;
  std::string mode = argc > 1 ? argv[1] : "";

  try {
    if (mode == "recv") {
      receiverMode(argc, argv);
    } else if (mode == "send") {
      sendMode(argc, argv);
    } else if (mode == "discv") {
      discoveryMode(argc, argv);
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
