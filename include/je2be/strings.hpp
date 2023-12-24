#pragma once

#include <je2be/integers.hpp>

#include <minecraft-file.hpp>

#include <charconv>
#include <cstdint>
#include <vector>

namespace je2be::strings {

inline std::u8string RemovePrefix(std::u8string_view const &s, std::u8string const &left) {
  if (left.empty()) {
    return std::u8string(s);
  }
  std::u8string ret(s);
  while (ret.starts_with(left)) {
    ret = ret.substr(left.size());
  }
  return ret;
}

inline std::u8string RemoveSuffix(std::u8string_view const &s, std::u8string const &right) {
  if (right.empty()) {
    return std::u8string(s);
  }
  std::u8string ret(s);
  while (ret.ends_with(right)) {
    ret = ret.substr(0, ret.size() - right.size());
  }
  return ret;
}

inline std::string RemoveSuffix(std::string_view const &s, std::string const &right) {
  if (right.empty()) {
    return std::string(s);
  }
  std::string ret(s);
  while (ret.ends_with(right)) {
    ret = ret.substr(0, ret.size() - right.size());
  }
  return ret;
}

inline std::u8string RemovePrefixAndSuffix(std::u8string const &left, std::u8string_view const &s, std::u8string const &right) {
  return RemoveSuffix(RemovePrefix(s, left), right);
}

inline std::u8string LTrim(std::u8string const &s) {
  std::u8string ws = u8" \r\n\t";
  auto start = s.find_first_not_of(ws);
  if (start == std::u8string::npos) {
    return {};
  } else {
    return s.substr(start);
  }
}

inline std::u8string RTrim(std::u8string const &s) {
  std::u8string ws = u8" \r\n\t";
  auto end = s.find_last_not_of(ws);
  if (end == std::u8string::npos) {
    return {};
  } else {
    return s.substr(0, end + 1);
  }
}

inline std::u8string Trim(std::u8string const &s) {
  return RTrim(LTrim(s));
}

inline std::u8string Remove(std::u8string_view const &s, std::u8string const &search) {
  if (search.empty()) {
    return std::u8string(s);
  }
  std::u8string ret(s);
  while (true) {
    auto i = ret.find(search);
    if (i == std::u8string::npos) {
      break;
    }
    ret = ret.substr(0, i) + ret.substr(i + search.size());
  }
  return ret;
}

inline std::u8string Replace(std::u8string_view const &target, std::u8string const &search, std::u8string const &replace) {
  using namespace std;
  if (search.empty()) {
    return u8string(target);
  }
  if (search == replace) {
    return u8string(target);
  }
  size_t offset = 0;
  u8string ret(target);
  while (true) {
    auto found = ret.find(search, offset);
    if (found == u8string::npos) {
      break;
    }
    ret = ret.substr(0, found) + replace + ret.substr(found + search.size());
    offset = found + replace.size();
  }
  return ret;
}

inline std::optional<i32> ToI32(std::u8string_view const &s, int base = 10) {
  i32 v = 0;
  if (auto [ptr, ec] = std::from_chars((char const *)s.data(), (char const *)s.data() + s.size(), v, base); ec == std::errc{}) {
    return v;
  } else {
    return std::nullopt;
  }
}

inline std::optional<i64> ToI64(std::u8string_view const &s, int base = 10) {
  i64 v = 0;
  if (auto [ptr, ec] = std::from_chars((char const *)s.data(), (char const *)s.data() + s.size(), v, base); ec == std::errc{}) {
    return v;
  } else {
    return std::nullopt;
  }
}

inline std::optional<u64> ToU64(std::u8string_view const &s, int base = 10) {
  u64 v = 0;
  if (auto [ptr, ec] = std::from_chars((char const *)s.data(), (char const *)s.data() + s.size(), v, base); ec == std::errc{}) {
    return v;
  } else {
    return std::nullopt;
  }
}

inline std::optional<float> ToF32(std::string_view const &s) {
  float v = 0;
#if defined(_MSC_VER)
  if (auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), v); ec == std::errc{}) {
    return v;
  } else {
    return std::nullopt;
  }
#else
  std::string tmp(s);
  if (std::sscanf(tmp.c_str(), "%f", &v) == 1) {
    return v;
  } else {
    return std::nullopt;
  }
#endif
}

inline bool Iequals(std::string_view const &a, std::string_view const &b) {
  if (a.size() != b.size()) {
    return false;
  }
  return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char ca, char cb) {
    return tolower(ca) == tolower(cb);
  });
}

inline void Split(std::u8string const &s, std::u8string const &delimiter, std::function<bool(int, std::u8string const &)> callback) {
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

inline std::u8string Capitalize(std::u8string const &s) {
  if (s.empty()) {
    return s;
  }
  return std::u8string(1, (char8_t)std::toupper(s[0])) + s.substr(1);
}

inline std::u8string CapitalizeSnake(std::u8string const &s) {
  std::u8string ret;
  Split(s, u8"_", [&ret](int i, std::u8string const &token) {
    if (i > 0) {
      ret += u8"_";
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

inline std::u8string UpperCamelFromSnake(std::u8string const &s) {
  auto tokens = mcfile::String::Split(s, u8'_');
  std::u8string ret;
  for (auto const &t : tokens) {
    ret += Capitalize(t);
  }
  return ret;
}

inline std::u8string SnakeFromUpperCamel(std::u8string const &s) {
  std::vector<std::u8string> tokens = mcfile::String::Split(s, u8'_');
  std::vector<std::u8string> waves;
  for (size_t i = 0; i < tokens.size(); i++) {
    auto token = tokens[i];
    if (token.empty()) {
      waves.push_back(token);
      continue;
    }
    std::u8string wave;
    for (size_t j = 0; j < token.size(); j++) {
      char8_t ch = token[j];
      char8_t lower = (char8_t)std::tolower(ch);
      if (lower == ch) {
        wave.push_back(ch);
      } else {
        if (!wave.empty()) {
          wave.push_back(u8'_');
        }
        wave.push_back(lower);
      }
    }
    waves.push_back(wave);
  }
  std::u8string ret;
  for (size_t i = 0; i < waves.size(); i++) {
    if (ret.empty()) {
      ret += waves[i];
    } else {
      ret += u8"_" + waves[i];
    }
  }
  return ret;
}

inline std::u8string Substring(std::u8string const &s, size_t begin, size_t end = std::u8string::npos) {
  return s.substr(begin, end - begin);
}

inline std::string Increment(std::string const &p) {
  using namespace std;
  if (p.empty()) {
    string s;
    s.push_back((char)1);
    return s;
  }
  vector<u16> v;
  for (int i = p.size() - 1; i >= 0; i--) {
    char ch = p[i];
    u8 a = *(u8 *)&ch;
    v.push_back(a);
  }
  v[0] += 1;
  for (int i = 0; i < v.size(); i++) {
    if (v[i] > std::numeric_limits<u8>::max()) {
      if (i == v.size() - 1) {
        v[i] = 0;
        v.push_back(1);
        break;
      } else {
        v[i + 1] += 1;
        v[i] = 0;
      }
    }
  }
  string ret;
  for (int i = v.size() - 1; i >= 0; i--) {
    u16 ch = v[i];
    ret.push_back(*(unsigned char *)&ch);
  }
  return ret;
}

inline std::u8string Unquote(std::u8string const &s, char8_t quoter) {
  if (s.starts_with(quoter) && s.ends_with(quoter)) {
    return s.substr(1, s.size() - 2);
  } else {
    return s;
  }
}

} // namespace je2be::strings
