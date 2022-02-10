#pragma once

namespace je2be::strings {

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

inline std::string Trim(std::string const &left, std::string const &s, std::string const &right) { return RTrim(LTrim(s, left), right); }

inline std::string Remove(std::string const &s, std::string const &search) {
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

inline std::string Replace(std::string const &target, std::string const &search, std::string const &replace) {
  using namespace std;
  if (search.empty()) {
    return target;
  }
  if (search == replace) {
    return target;
  }
  size_t offset = 0;
  string ret = target;
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

inline std::optional<int32_t> Toi(std::string const &s, int base = 10) {
  int32_t v = 0;
  if (auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), v, base); ec == std::errc{}) {
    return v;
  } else {
    return std::nullopt;
  }
}

inline std::optional<int64_t> Tol(std::string const &s, int base = 10) {
  int64_t v = 0;
  if (auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), v, base); ec == std::errc{}) {
    return v;
  } else {
    return std::nullopt;
  }
}

inline bool Iequals(std::string const &a, std::string const &b) {
  if (a.size() != b.size()) {
    return false;
  }
  return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char ca, char cb) {
    return tolower(ca) == tolower(cb);
  });
}

inline std::string Capitalize(std::string const &s) {
  if (s.empty()) {
    return s;
  }
  return std::string(1, (char)std::toupper((unsigned char)s[0])) + s.substr(1);
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

} // namespace je2be::strings
