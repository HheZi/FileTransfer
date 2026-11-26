#ifndef COMMON_FILE_SIZE_CONVERTER_HPP
#define COMMON_FILE_SIZE_CONVERTER_HPP

#include <format>
#include <cstdint>
#include <string>

class FileSizeConvert {
  constexpr static std::uint64_t TERABYTE_IN_BYTES = 1099511627776;
  constexpr static std::uint64_t GIGABYTE_IN_BYTES = 1073741824;
  constexpr static std::uint32_t MEGABYTE_IN_BYTES = 1048576;
  constexpr static std::uint16_t KILOBYTE_IN_BYTES = 1024;

public:
static std::string toHumanReadable(const std::uint64_t file_size_in_bytes) {
    const double file_size = static_cast<double>(file_size_in_bytes);

    if (file_size >= TERABYTE_IN_BYTES) {
        return std::format("{:.2f} TB", file_size / TERABYTE_IN_BYTES);
    } else if (file_size >= GIGABYTE_IN_BYTES) {
        return std::format("{:.2f} GB", file_size / GIGABYTE_IN_BYTES);
    } else if (file_size >= MEGABYTE_IN_BYTES) {
        return std::format("{:.2f} MB", file_size / MEGABYTE_IN_BYTES);
    } else if (file_size >= KILOBYTE_IN_BYTES) {
        return std::format("{:.2f} KB", file_size / KILOBYTE_IN_BYTES);
    }
    return std::format("{} B", file_size_in_bytes);
}
};

#endif
