#pragma once

namespace j2b::strings {

inline bool Equal(std::string const &a, std::string const &b) {
  if (a.size() != b.size()) {
    return false;
  }
  return memcmp(a.c_str(), b.c_str(), a.size()) == 0;
}

inline std::string LTrim(std::string const &s, std::string const &left) {
  if (left.empty()) {
    return s;
  }
  std::string ret = s;
  while (ret.starts_with(left)) {
    ret = ret.substr(left.size());
  }
  return ret;
}

inline std::string RTrim(std::string const &s, std::string const &right) {
  if (right.empty()) {
    return s;
  }
  std::string ret = s;
  while (ret.ends_with(right)) {
    ret = ret.substr(0, ret.size() - right.size());
  }
  return ret;
}

inline std::string Trim(std::string const &left, std::string const &s,
                        std::string const &right) {
  return RTrim(LTrim(s, left), right);
}

inline std::string Replace(std::string const &s, std::string const &search) {
  if (search.empty()) {
    return s;
  }
  std::string ret = s;
  while (true) {
    auto i = ret.find(search);
    if (i == std::string::npos) {
      break;
    }
    ret = ret.substr(0, i) + ret.substr(i + search.size());
  }
  return ret;
}

inline std::optional<int32_t> Toi(std::string const &s, int base = 10) {
  try {
    return std::stoi(s, nullptr, base);
  } catch (...) {
    return std::nullopt;
  }
}

inline std::optional<int64_t> Tol(std::string const &s, int base = 10) {
  try {
    return std::stoll(s, nullptr, base);
  } catch (...) {
    return std::nullopt;
  }
}

} // namespace j2b::strings
