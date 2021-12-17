#pragma once

namespace je2be {
struct Uuid {
  uint32_t f1;
  uint32_t f2;
  uint32_t f3;
  uint32_t f4;
};

struct UuidHasher {
  size_t operator()(Uuid const &k) const {
    size_t res = 17;
    res = res * 31 + std::hash<uint32_t>{}(k.f1);
    res = res * 31 + std::hash<uint32_t>{}(k.f2);
    res = res * 31 + std::hash<uint32_t>{}(k.f3);
    res = res * 31 + std::hash<uint32_t>{}(k.f4);
    return res;
  };
};

struct UuidPred {
  bool operator()(Uuid const &a, Uuid const &b) const {
    return a.f1 == b.f1 &&
           a.f2 == b.f2 &&
           a.f3 == b.f3 &&
           a.f4 == b.f4;
  }
};
} // namespace je2be
