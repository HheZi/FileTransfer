#include <string>
#include <random>
#include <cstdio>
#include <file_transfer/common/id_generator.hpp>

std::string IdGenerator::generate() {
  std::random_device rd;
  std::uniform_int_distribution<unsigned long long> dist;
  unsigned long long v = (static_cast<unsigned long long>(rd()) << 32) ^ rd();
  char buf[32];
  std::snprintf(buf, sizeof buf, "%016llx", v);
  return buf;
}
