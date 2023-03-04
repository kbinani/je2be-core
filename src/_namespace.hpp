#pragma once

namespace je2be {
class Namespace {
  Namespace() = delete;

public:
  static std::string_view Remove(std::string_view const &v) {
    return Remove<std::string_view>(v);
  }

  static std::string Remove(std::string const &v) {
    return Remove<std::string>(v);
  }

  static std::string Add(std::string const &v) {
    if (v.starts_with("minecraft:")) {
      return v;
    } else {
      return "minecraft:" + v;
    }
  }

private:
  template <class T>
  static T Remove(T const &v) {
    if (v.starts_with("minecraft:")) {
      return v.substr(10);
    } else {
      return v;
    }
  }
};
} // namespace je2be
