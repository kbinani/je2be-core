#pragma once

namespace je2be {
class Namespace {
  Namespace() = delete;

public:
  static std::u8string_view Remove(std::u8string_view const &v) {
    return Remove<std::u8string_view>(v);
  }

  static std::u8string Remove(std::u8string const &v) {
    return Remove<std::u8string>(v);
  }

  static std::u8string Add(std::u8string const &v) {
    if (v.starts_with(u8"minecraft:")) {
      return v;
    } else {
      return u8"minecraft:" + v;
    }
  }

private:
  template <class T>
  static T Remove(T const &v) {
    if (v.starts_with(u8"minecraft:")) {
      return v.substr(10);
    } else {
      return v;
    }
  }
};
} // namespace je2be
