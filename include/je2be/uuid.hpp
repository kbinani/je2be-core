#pragma once

namespace je2be {
struct Uuid {
  alignas(32) uint8_t fData[16];

  static Uuid Gen() {
    std::random_device r;
    return GenWithSeed(r());
  }

  static Uuid FromData(uint8_t data[16]) {
    Uuid u;
    for (int i = 0; i < 16; i++) {
      u.fData[i] = data[i];
    }
    return u;
  }

  static Uuid FromInt32(int32_t f1, int32_t f2, int32_t f3, int32_t f4) {
    Uuid u;
    *(uint32_t *)u.fData = *(uint32_t *)&f1;
    *((uint32_t *)u.fData + 1) = *(uint32_t *)&f2;
    *((uint32_t *)u.fData + 2) = *(uint32_t *)&f3;
    *((uint32_t *)u.fData + 3) = *(uint32_t *)&f4;
    return u;
  }

  static std::optional<Uuid> FromString(std::string const &str) {
    using namespace std;
    string uuid = strings::Replace(str, "-", "");
    if (uuid.size() != 32) {
      return nullopt;
    }
    uint8_t data[16];
    for (int i = 0; i < 16; i++) {
      auto sub = uuid.substr(i * 2, 2);
      auto converted = strings::Toi(sub, 16);
      if (!converted) {
        return nullopt;
      }
      data[i] = 0xff & ((uint32_t)*converted);
    }
    return Uuid::FromData(data);
  }

  static Uuid GenWithSeed(size_t seed) {
    std::mt19937 mt(seed);
    return GenWithGenerator(mt);
  }

  template <class Generator>
  static Uuid GenWithGenerator(Generator &g) {
    std::uniform_int_distribution<uint32_t> distribution;

    Uuid ret;
    *(uint32_t *)ret.fData = distribution(g);
    *((uint32_t *)ret.fData + 1) = distribution(g);
    *((uint32_t *)ret.fData + 2) = distribution(g);
    *((uint32_t *)ret.fData + 3) = distribution(g);

    // Variant
    ret.fData[8] &= 0xBF;
    ret.fData[8] |= 0x80;

    // Version
    ret.fData[6] &= 0x4F;
    ret.fData[6] |= 0x40;

    return ret;
  }

  static Uuid GenWithU64Seed(uint64_t seed) {
    std::mt19937_64 mt(seed);
    return GenWithGenerator(mt);
  }

  static Uuid GenWithI64Seed(int64_t seed) {
    uint64_t u = *(uint64_t *)&seed;
    return GenWithU64Seed(u);
  }

  std::string toString() const {
    std::ostringstream s;
    s << std::hex << std::setfill('0')
      << std::setw(2) << (int)fData[0]
      << std::setw(2) << (int)fData[1]
      << std::setw(2) << (int)fData[2]
      << std::setw(2) << (int)fData[3]
      << '-'
      << std::setw(2) << (int)fData[4]
      << std::setw(2) << (int)fData[5]
      << '-'
      << std::setw(2) << (int)fData[6]
      << std::setw(2) << (int)fData[7]
      << '-'
      << std::setw(2) << (int)fData[8]
      << std::setw(2) << (int)fData[9]
      << '-'
      << std::setw(2) << (int)fData[10]
      << std::setw(2) << (int)fData[11]
      << std::setw(2) << (int)fData[12]
      << std::setw(2) << (int)fData[13]
      << std::setw(2) << (int)fData[14]
      << std::setw(2) << (int)fData[15];
    return s.str();
  }

  ListTagPtr toListTag() const {
    auto ret = List<Tag::Type::Int>();
    int32_t v1 = mcfile::I32FromBE(*(int32_t *)fData);
    int32_t v2 = mcfile::I32FromBE(*((int32_t *)fData + 1));
    int32_t v3 = mcfile::I32FromBE(*((int32_t *)fData + 2));
    int32_t v4 = mcfile::I32FromBE(*((int32_t *)fData + 3));
    ret->push_back(std::make_shared<IntTag>(v1));
    ret->push_back(std::make_shared<IntTag>(v2));
    ret->push_back(std::make_shared<IntTag>(v3));
    ret->push_back(std::make_shared<IntTag>(v4));
    return ret;
  }

  IntArrayTagPtr toIntArrayTag() const {
    int32_t v1 = mcfile::I32FromBE(*(int32_t *)fData);
    int32_t v2 = mcfile::I32FromBE(*((int32_t *)fData + 1));
    int32_t v3 = mcfile::I32FromBE(*((int32_t *)fData + 2));
    int32_t v4 = mcfile::I32FromBE(*((int32_t *)fData + 3));
    std::vector<int32_t> uuidValues({v1, v2, v3, v4});
    return std::make_shared<IntArrayTag>(uuidValues);
  }

  static std::optional<Uuid> FromIntArrayTag(IntArrayTag const &tag) {
    auto const &values = tag.value();
    if (values.size() != 4) {
      return std::nullopt;
    }
    int32_t f1 = values[0];
    int32_t f2 = values[1];
    int32_t f3 = values[2];
    int32_t f4 = values[3];
    Uuid u;
    *(int32_t *)u.fData = mcfile::I32FromBE(values[0]);
    *((int32_t *)u.fData + 1) = mcfile::I32FromBE(values[1]);
    *((int32_t *)u.fData + 2) = mcfile::I32FromBE(values[2]);
    *((int32_t *)u.fData + 3) = mcfile::I32FromBE(values[3]);
    return u;
  }
};

struct UuidHasher {
  size_t operator()(Uuid const &k) const {
    size_t res = 17;
    for (int i = 0; i < 16; i++) {
      res = res * 31 + std::hash<uint8_t>{}(k.fData[i]);
    }
    return res;
  };
};

struct UuidPred {
  bool operator()(Uuid const &a, Uuid const &b) const {
    for (int i = 0; i < 16; i++) {
      if (a.fData[i] != b.fData[i]) {
        return false;
      }
    }
    return true;
  }
};
} // namespace je2be
