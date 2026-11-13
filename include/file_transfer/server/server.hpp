#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include <cstdint>
#include <string>
#include <boost/asio.hpp>
#include "client_connection_handler.hpp"
#include "server_discovery.hpp"

using tcp = boost::asio::ip::tcp;

class Server {
public:  
  Server(std::uint32_t port, const std::string working_dir, std::string id,
                  std::string name);

  void start();

  void stop();

  void setDecider(ClientConnectionHandler::Decider decider);

  void setFileProgress(ClientConnectionHandler::FileReceiveProgress file_progress);  

private:
  boost::asio::io_context io_context_;
  tcp::acceptor acceptor_;
  std::string working_dir_;

  ClientConnectionHandler::Decider decider_;
  ClientConnectionHandler::FileReceiveProgress file_progress_;  

  ServerDiscovery server_discovery_;

  void createWorkingDirectoryIfNotExists();

  void doAccept();

  void handleAccept(ClientConnectionHandler::connection_ptr conn,
                    const boost::system::error_code &error);
};

#endif
