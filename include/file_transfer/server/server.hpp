#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <cstdint>
#include <string>
#include <boost/asio.hpp>
#include "client_connection_handler.hpp"
#include "server_discovery.hpp"

class Server {
public:
  using tcp = boost::asio::ip::tcp;

  explicit Server(std::uint32_t port, const std::string working_dir, std::string id,
                  std::string name);

  void start();

private:
  boost::asio::io_context io_context_;
  tcp::acceptor acceptor_;
  std::string working_dir_;

  ServerDiscovery server_discovery_;

  void createWorkingDirectoryIfNotExists();

  void doAccept();

  void handleAccept(ClientConnectionHandler::connection_ptr conn,
                    const boost::system::error_code &error);
};

#endif
