#ifndef SERVER_CLIENT_CONNECTION_HANDLER_HPP
#define SERVER_CLIENT_CONNECTION_HANDLER_HPP

#include "FileOffer.pb.h"
#include <cstdint>
#include <string>
#include <filesystem>
#include <memory>
#include <boost/asio.hpp>

using tcp = boost::asio::ip::tcp;
namespace fs = std::filesystem;

struct FileInfo {
  std::string name;
  std::uint64_t size;

  FileInfo() = default;
  FileInfo(FileOffer &offer) {
    name = offer.name();
    size = offer.size();
  }
};

class ClientConnectionHandler
    : public std::enable_shared_from_this<ClientConnectionHandler> {
public:
  using connection_ptr = std::shared_ptr<ClientConnectionHandler>;
  using Decider = std::function<bool(const FileInfo &)>;
  using FileReceiveProgress = std::function<void(const std::uint64_t total, const std::uint64_t got)>;

  static connection_ptr create(boost::asio::io_context &io_context,
                               std::string &working_dir, Decider &decider,
                               FileReceiveProgress& file_progress);

  tcp::socket &socket();

  void start();

private:
  static constexpr std::size_t CHUNK_SIZE = 64 * 1024;

  ClientConnectionHandler(boost::asio::io_context &io_context,
                          std::string &working_dir, Decider &decider,
                          FileReceiveProgress& file_progress);
  tcp::socket socket_;
  fs::path dir_;
  Decider &decider_;
  FileReceiveProgress& file_progress_;  

  void acceptFile(const std::string &fileName, const std::uint64_t size);

  FileAnswer decide(FileOffer &offer);
};

#endif
