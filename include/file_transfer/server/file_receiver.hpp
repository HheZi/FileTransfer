#ifndef SERVER_FILE_RECEIVER_HPP
#define SERVER_FILE_RECEIVER_HPP

#include <cstdint>
#include <fstream>
#include <generator>
#include <filesystem>
#include <boost/asio.hpp>
#include <memory>
#include <string>

class FileReceiver : public std::enable_shared_from_this<FileReceiver> {
public:
  ~FileReceiver();
  static std::shared_ptr<FileReceiver>
  create(boost::asio::ip::tcp::socket &socket, const std::filesystem::path dir,
         const std::string file_name, const std::uint64_t file_size);

  std::string file_name;
  std::uint64_t file_size;

  std::generator<std::uint64_t> receiveFile();

  void stop();

private:
  FileReceiver(boost::asio::ip::tcp::socket &socket,
               const std::filesystem::path dir, const std::string file_name,
               const std::uint64_t file_size);

  boost::asio::ip::tcp::socket socket_;

  std::filesystem::path dir_;

  bool is_stoped_;

  std::shared_ptr<std::ofstream> out_;

  void closeConnection();

  void deleteFile();
};

#endif
