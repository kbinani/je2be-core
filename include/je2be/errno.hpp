#pragma once

#include <string>
#include <vector>

namespace je2be {

class Errno {
  Errno() = delete;

public:
  static std::string StringFromErrno(decltype(errno) e) {
#if defined(_MSC_VER)
    std::vector<char> buffer(94 + 1, (char)0);
    if (strerror_s(buffer.data(), buffer.size(), e) == 0) {
#else
    std::vector<char> buffer(256, (char)0);
    if (strerror_r(e, buffer.data(), buffer.size()) == 0) {
#endif
      return std::string(buffer.data());
    } else {
      return "(strerror_s failed: errno=" + std::to_string(e) + ")";
    }
  }
};

} // namespace je2be
