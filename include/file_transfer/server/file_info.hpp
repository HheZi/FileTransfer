#ifndef SERVER_FILE_INFO_HPP
#define SERVER_FILE_INFO_HPP

#include <string>
#include <cstdint>
#include "FileOffer.pb.h"

struct FileInfo {
  std::string name;
  std::uint64_t size;

  FileInfo() = default;
  FileInfo(FileOffer &offer) : name(offer.name()), size(offer.size()) {}
};

#endif
