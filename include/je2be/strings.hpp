#pragma once

#include <minecraft-file.hpp>

namespace je2be::strings {

inline std::string LTrim(std::string_view const &s, std::string const &left) {
  if (left.empty()) {
    return std::string(s);
  }
  std::string ret(s);
  while (ret.starts_with(left)) {
    ret = ret.substr(left.size());
  }
  return ret;
}

inline std::string RTrim(std::string_view const &s, std::string const &right) {
  if (right.empty()) {
    return std::string(s);
  }
  std::string ret(s);
  while (ret.ends_with(right)) {
    ret = ret.substr(0, ret.size() - right.size());
  }
  return ret;
}

inline std::string Trim(std::string const &left, std::string_view const &s, std::string const &right) { return RTrim(LTrim(s, left), right); }

inline std::string Remove(std::string_view const &s, std::string const &search) {
  if (search.empty()) {
    return std::string(s);
  }
  std::string ret(s);
  while (true) {
    auto i = ret.find(search);
    if (i == std::string::npos) {
      break;
    }
    ret = ret.substr(0, i) + ret.substr(i + search.size());
  }
  return ret;
}

inline std::string Replace(std::string_view const &target, std::string const &search, std::string const &replace) {
  using namespace std;
  if (search.empty()) {
    return string(target);
  }
  if (search == replace) {
    return string(target);
  }
  size_t offset = 0;
  string ret(target);
  while (true) {
    auto found = ret.find(search, offset);
    if (found == string::npos) {
      break;
    }
    ret = ret.substr(0, found) + replace + ret.substr(found + search.size());
    offset = found + replace.size();
  }
  return ret;
}

inline std::optional<int32_t> Toi(std::string_view const &s, int base = 10) {
  int32_t v = 0;
  if (auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), v, base); ec == std::errc{}) {
    return v;
  } else {
    return std::nullopt;
  }
}

inline std::optional<int64_t> Tol(std::string_view const &s, int base = 10) {
  int64_t v = 0;
  if (auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), v, base); ec == std::errc{}) {
    return v;
  } else {
    return std::nullopt;
  }
}

inline std::optional<float> Tof(std::string_view const &s) {
  float v = 0;
  if (auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), v); ec == std::errc{}) {
    return v;
  } else {
    return std::nullopt;
  }
}

inline bool Iequals(std::string_view const &a, std::string_view const &b) {
  if (a.size() != b.size()) {
    return false;
  }
  return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char ca, char cb) {
    return tolower(ca) == tolower(cb);
  });
}

inline void Split(std::string const &s, std::string const &delimiter, std::function<bool(int, std::string const &)> callback) {
  if (delimiter.empty()) {
    callback(0, s);
    return;
  }
  size_t off = 0;
  size_t idx = s.find(delimiter, off);
  int count = 0;
  while (idx != std::string::npos) {
    callback(count, s.substr(off, idx - off));
    count++;
    off = idx + delimiter.size();
    idx = s.find(delimiter, off);
  }
  callback(count, s.substr(off));
}

inline std::string Capitalize(std::string const &s) {
  if (s.empty()) {
    return s;
  }
  return std::string(1, (char)std::toupper((unsigned char)s[0])) + s.substr(1);
}

inline std::string CapitalizeSnake(std::string const &s) {
  std::string ret;
  Split(s, "_", [&ret](int i, std::string const &token) {
    if (i > 0) {
      ret += "_";
    }
    ret += Capitalize(token);
    return true;
  });
  return ret;
}

inline std::string Uncapitalize(std::string const &s) {
  if (s.empty()) {
    return s;
  }
  return std::string(1, (char)std::tolower((unsigned char)s[0])) + s.substr(1);
}

inline std::string UpperCamelFromSnake(std::string const &s) {
  auto tokens = mcfile::String::Split(s, '_');
  std::string ret;
  for (auto const &t : tokens) {
    ret += Capitalize(t);
  }
  return ret;
}

inline std::string SnakeFromUpperCamel(std::string const &s) {
  std::string ret;
  for (size_t i = 0; i < s.size(); i++) {
    char ch = s[i];
    char lower = (char)std::tolower((unsigned char)ch);
    if (lower == ch) {
      ret.push_back(ch);
    } else {
      if (!ret.empty()) {
        ret.push_back('_');
      }
      ret.push_back(lower);
    }
  }
  return ret;
}

inline std::string Substring(std::string const &s, size_t begin, size_t end = std::string::npos) {
  return s.substr(begin, end - begin);
}

} // namespace je2be::strings
