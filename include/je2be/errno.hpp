#pragma once

#include <string.h>

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
      return std::string(buffer.data());
    } else {
      return "(strerror_s failed: errno=" + std::to_string(e) + ")";
    }
#elif ((_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE) || defined(__EMSCRIPTEN__) || defined(__APPLE__)
    // XSI-compatible
    std::vector<char> buffer(256, (char)0);
    if (int ret = strerror_r(e, buffer.data(), buffer.size()); ret == 0) {
      return std::string(buffer.data());
    } else {
      return "(strerror_r failed: errno=" + std::to_string(e) + ")";
    }
#else
    // GNU-specific
    std::vector<char> buffer(256, (char)0);
    if (char *ptr = strerror_r(e, buffer.data(), buffer.size()); ptr) {
      return std::string(ptr);
    } else {
      return "(strerror_r failed: errno=" + std::to_string(e) + ")";
    }
#endif
  }
};

} // namespace je2be
