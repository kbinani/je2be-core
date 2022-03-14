#pragma once

namespace je2be {

class defer_t {
public:
  defer_t(std::function<void(void)> f)
      : f(f) {}

  ~defer_t() {
    try {
      f();
    } catch (...) {
    }
  }

private:
  std::function<void(void)> f;
};

} // namespace je2be

#define defer_helper2(line) defer_tmp##line
#define defer_helper(line) defer_helper2(line)
#define defer je2be::defer_t defer_helper(__LINE__) = [&](void) -> void
