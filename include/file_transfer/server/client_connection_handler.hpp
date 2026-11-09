#ifndef SERVER_CLIENT_CONNECTION_HANDLER_HPP
#define SERVER_CLIENT_CONNECTION_HANDLER_HPP

#include "FileOffer.pb.h"
#include <cstdint>
#include <string>
#include <filesystem>
#include <memory>
#include <boost/asio.hpp>

namespace fs = std::filesystem;

class ClientConnectionHandler
    : public std::enable_shared_from_this<ClientConnectionHandler> {
public:
  using tcp = boost::asio::ip::tcp;
  using connection_ptr = std::shared_ptr<ClientConnectionHandler>;

  static connection_ptr create(boost::asio::io_context &io_context,
                               std::string &working_dir);

  tcp::socket &socket();

  void start();

private:
  static constexpr std::size_t CHUNK_SIZE = 64 * 1024;
  
  ClientConnectionHandler(boost::asio::io_context &io_context,
                          std::string &working_dir);
  tcp::socket socket_;
  fs::path dir_;

  void acceptFile(const std::string &fileName, const std::uint64_t size);

  FileAnswer decide(FileOffer &offer);
};

#endif
