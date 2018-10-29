#ifndef UTIL_FILE_IO_HPP_
#define UTIL_FILE_IO_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <limits>

namespace util {

class file_io {
 public:
  static file_io& get() {
    static file_io instance;
    return instance;
  }

  // ...
  std::string read(std::string_view filename) {
    std::stringstream ss;
    try {
      std::ifstream file;
      file.open(std::string(filename), std::ios::in);
      if (!file.fail()) {
        ss << file.rdbuf();
      }
      file.close();
    } catch (const std::exception& e) {
      std::cerr << "ERROR: Cannot open the file: " << filename << " | "
            << e.what() << std::endl;
    }
    return ss.str();
  }

  template <typename T>
  void save(T data, std::string_view filename) {
    try {
        std::ofstream file;
        file.open(std::string(filename), std::ios::app);
        file.precision(std::numeric_limits<double>::max_digits10);
        file << data << "\n";
        file.close();
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Cannot open the file: " << filename << " | "
            << e.what() << std::endl;
    }
  }
};

}  // namespace util

#endif  // UTIL_FILE_IO_HPP_
