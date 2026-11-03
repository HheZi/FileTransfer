#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>
#include <boost/asio.hpp>
#include "client_connection_handler.cpp"
#include "server_discovery.cpp"

class Server {
public:
  using tcp = boost::asio::ip::tcp;

  explicit Server(std::uint32_t port, const char *working_dir, std::string id,
                  std::string name)
      : io_context_(), acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)),
        working_dir_(working_dir),
        server_discovery_(io_context_, id, port, name) {

    createWorkingDirectoryIfNotExists();
  }

  void start() {
    std::cout << "Server has started" << std::endl;

    server_discovery_.start();
    doAccept();

    io_context_.run();
  }

private:
  boost::asio::io_context io_context_;
  tcp::acceptor acceptor_;
  std::string working_dir_;

  ServerDiscovery server_discovery_;

  void createWorkingDirectoryIfNotExists() {
    if (!std::filesystem::exists(working_dir_)) {
      std::filesystem::create_directories(working_dir_);
    }
  }

  void doAccept() {
    ClientConnectionHandler::connection_ptr conn =
        ClientConnectionHandler::create(io_context_, working_dir_);

    acceptor_.async_accept(conn->socket(),
                           [this, conn](const boost::system::error_code &ec) {
                             handleAccept(conn, ec);
                           });
  }

  void handleAccept(ClientConnectionHandler::connection_ptr conn,
                    const boost::system::error_code &error) {
    if (!error) {
      conn->start();
    } else {
      std::cerr << error.message() << std::endl;
    }

    doAccept();
  }
};
