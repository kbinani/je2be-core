#pragma once

namespace je2be {
struct Uuid {
  uint32_t f1;
  uint32_t f2;
  uint32_t f3;
  uint32_t f4;

  static Uuid Gen() {
    std::random_device r;
    return GenWithSeed(r());
  }

  static Uuid GenWithSeed(size_t seed) {
    std::uniform_int_distribution<uint32_t> distribution;
    std::mt19937 mt(seed);

    Uuid ret;
    ret.f1 = distribution(mt);
    ret.f2 = distribution(mt);
    ret.f3 = distribution(mt);
    ret.f4 = distribution(mt);

    // Variant
    *((uint8_t *)&ret.f1 + 8) &= 0xBF;
    *((uint8_t *)&ret.f1 + 8) |= 0x80;

    // Version
    *((uint8_t *)&ret.f1 + 6) &= 0x4F;
    *((uint8_t *)&ret.f1 + 6) |= 0x40;

    return ret;
  }

  static Uuid GenWithU64Seed(uint64_t seed) {
    std::uniform_int_distribution<uint32_t> distribution;
    std::mt19937_64 mt(seed);

    Uuid ret;
    ret.f1 = distribution(mt);
    ret.f2 = distribution(mt);
    ret.f3 = distribution(mt);
    ret.f4 = distribution(mt);

    // Variant
    *((uint8_t *)&ret.f1 + 8) &= 0xBF;
    *((uint8_t *)&ret.f1 + 8) |= 0x80;

    // Version
    *((uint8_t *)&ret.f1 + 6) &= 0x4F;
    *((uint8_t *)&ret.f1 + 6) |= 0x40;

    return ret;
  }

  std::string toString() const {
    std::ostringstream s;
    s << std::hex << std::setfill('0')
      << std::setw(2) << (int)(*((uint8_t *)&f1 + 0))
      << std::setw(2) << (int)(*((uint8_t *)&f1 + 1))
      << std::setw(2) << (int)(*((uint8_t *)&f1 + 2))
      << std::setw(2) << (int)(*((uint8_t *)&f1 + 3))
      << '-'
      << std::setw(2) << (int)(*((uint8_t *)&f2 + 0))
      << std::setw(2) << (int)(*((uint8_t *)&f2 + 1))
      << '-'
      << std::setw(2) << (int)(*((uint8_t *)&f2 + 2))
      << std::setw(2) << (int)(*((uint8_t *)&f2 + 3))
      << '-'
      << std::setw(2) << (int)(*((uint8_t *)&f3 + 0))
      << std::setw(2) << (int)(*((uint8_t *)&f3 + 1))
      << '-'
      << std::setw(2) << (int)(*((uint8_t *)&f2 + 2))
      << std::setw(2) << (int)(*((uint8_t *)&f2 + 3))
      << std::setw(2) << (int)(*((uint8_t *)&f3 + 0))
      << std::setw(2) << (int)(*((uint8_t *)&f3 + 1))
      << std::setw(2) << (int)(*((uint8_t *)&f3 + 2))
      << std::setw(2) << (int)(*((uint8_t *)&f3 + 3));
    return s.str();
  }

  std::shared_ptr<ListTag> toListTag() const {
    auto ret = std::make_shared<ListTag>(Tag::Type::Int);
    ret->push_back(std::make_shared<IntTag>(*(int32_t *)&f1));
    ret->push_back(std::make_shared<IntTag>(*(int32_t *)&f2));
    ret->push_back(std::make_shared<IntTag>(*(int32_t *)&f3));
    ret->push_back(std::make_shared<IntTag>(*(int32_t *)&f4));
    return ret;
  }
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
