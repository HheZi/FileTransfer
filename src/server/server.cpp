#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>
#include <boost/asio.hpp>
#include "file_transfer/server/server.hpp"
#include "file_transfer/server/client_connection_handler.hpp"

Server::Server(std::uint32_t port, const std::string working_dir, std::string id,
               std::string name)
    : io_context_(), acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)),
      working_dir_(working_dir),
      server_discovery_(io_context_, id, port, name) {

  createWorkingDirectoryIfNotExists();
}

void Server::start() {
  std::cout << "Server has started" << std::endl;

  server_discovery_.start();
  doAccept();

  io_context_.run();
}

void Server::stop() {
  server_discovery_.stop();
  io_context_.stop();
}

void Server::setDecider(ClientConnectionHandler::Decider decider) {
  decider_ = decider;
}

void Server::setFileProgress(
    ClientConnectionHandler::FileReceiveProgress file_progress) {
  file_progress_ = file_progress;
}

void Server::createWorkingDirectoryIfNotExists() {
  if (!std::filesystem::exists(working_dir_)) {
    std::filesystem::create_directories(working_dir_);
  }
}

void Server::doAccept() {
  ClientConnectionHandler::connection_ptr conn =
      ClientConnectionHandler::create(io_context_, working_dir_, decider_,
                                      file_progress_);

  acceptor_.async_accept(conn->socket(),
                         [this, conn](const boost::system::error_code &ec) {
                           handleAccept(conn, ec);
                         });
}

void Server::handleAccept(ClientConnectionHandler::connection_ptr conn,
                          const boost::system::error_code &error) {
  if (!error) {
    conn->start();
  } else {
    std::cerr << error.message() << std::endl;
  }

  doAccept();
}
