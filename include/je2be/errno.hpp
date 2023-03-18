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
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
    if constexpr (std::is_same_v<std::invoke_result_t<decltype(strerror_r), int, char *, size_t>, int>) {
#pragma GCC diagnostic pop
      // XSI-compatible
      std::vector<char> buffer(256, (char)0);
      if (strerror_r(e, buffer.data(), buffer.size()) == 0) {
        return std::string(buffer.data());
      } else {
        return "(strerror_r failed: errno=" + std::to_string(e) + ")";
      }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
    } else if constexpr (std::is_same_v<std::invoke_result_t<decltype(strerror_r), int, char *, size_t>, char *>) {
#pragma GCC diagnostic pop
      // GNU-specific
      std::vector<char> buffer(256, (char)0);
      char *ptr = strerror_r(e, buffer.data(), buffer.size());
      if (ptr) {
        return std::string(ptr);
      } else {
        return "(strerror_r failed: errno=" + std::to_string(e) + ")";
      }
    } else {
      return "(strerror_r unavailable: errno=" + std::to_string(e) + ")";
    }
#endif
  }
};

} // namespace je2be
