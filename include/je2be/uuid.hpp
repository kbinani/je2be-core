#pragma once

#include <je2be/nbt.hpp>
#include <je2be/strings.hpp>

namespace je2be {
struct Uuid {
  alignas(64) u8 fData[16];

  Uuid() {
    std::fill_n(fData, 16, 0);
  }

  Uuid(Uuid const &other) {
    std::copy_n(other.fData, 16, fData);
  }

  Uuid &operator=(Uuid const &other) {
    if (this == &other) {
      return *this;
    }
    std::copy_n(other.fData, 16, fData);
    return *this;
  }

  static Uuid Gen() {
    std::random_device r;
    return GenWithSeed(r());
  }

  static Uuid FromData(u8 data[16]) {
    Uuid u;
    for (int i = 0; i < 16; i++) {
      u.fData[i] = data[i];
    }
    return u;
  }

  static Uuid FromInt32(i32 f1, i32 f2, i32 f3, i32 f4) {
    Uuid u;
    *(u32 *)u.fData = *(u32 *)&f1;
    *((u32 *)u.fData + 1) = *(u32 *)&f2;
    *((u32 *)u.fData + 2) = *(u32 *)&f3;
    *((u32 *)u.fData + 3) = *(u32 *)&f4;
    return u;
  }

  static std::optional<Uuid> FromString(std::string const &str) {
    using namespace std;
    string uuid = strings::Replace(str, "-", "");
    if (uuid.size() != 32) {
      return nullopt;
    }
    u8 data[16];
    for (int i = 0; i < 16; i++) {
      auto sub = uuid.substr(i * 2, 2);
      auto converted = strings::Toi(sub, 16);
      if (!converted) {
        return nullopt;
      }
      data[i] = 0xff & ((u32)*converted);
    }
    return Uuid::FromData(data);
  }

  static Uuid GenWithSeed(size_t seed) {
    std::mt19937 mt(seed);
    return GenWithGenerator(mt);
  }

  template <class Generator>
  static Uuid GenWithGenerator(Generator &g) {
    std::uniform_int_distribution<u32> distribution;

    Uuid ret;
    *(u32 *)ret.fData = distribution(g);
    *((u32 *)ret.fData + 1) = distribution(g);
    *((u32 *)ret.fData + 2) = distribution(g);
    *((u32 *)ret.fData + 3) = distribution(g);

    // Variant
    ret.fData[8] &= 0xBF;
    ret.fData[8] |= 0x80;

    // Version
    ret.fData[6] &= 0x4F;
    ret.fData[6] |= 0x40;

    return ret;
  }

  static Uuid GenWithU64Seed(u64 seed) {
    std::mt19937_64 mt(seed);
    return GenWithGenerator(mt);
  }

  static Uuid GenWithI64Seed(i64 seed) {
    u64 u = *(u64 *)&seed;
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
    i32 v1 = mcfile::I32FromBE(*(i32 *)fData);
    i32 v2 = mcfile::I32FromBE(*((i32 *)fData + 1));
    i32 v3 = mcfile::I32FromBE(*((i32 *)fData + 2));
    i32 v4 = mcfile::I32FromBE(*((i32 *)fData + 3));
    ret->push_back(std::make_shared<IntTag>(v1));
    ret->push_back(std::make_shared<IntTag>(v2));
    ret->push_back(std::make_shared<IntTag>(v3));
    ret->push_back(std::make_shared<IntTag>(v4));
    return ret;
  }

  IntArrayTagPtr toIntArrayTag() const {
    i32 v1 = mcfile::I32FromBE(*(i32 *)fData);
    i32 v2 = mcfile::I32FromBE(*((i32 *)fData + 1));
    i32 v3 = mcfile::I32FromBE(*((i32 *)fData + 2));
    i32 v4 = mcfile::I32FromBE(*((i32 *)fData + 3));
    std::vector<i32> uuidValues({v1, v2, v3, v4});
    return std::make_shared<IntArrayTag>(uuidValues);
  }

  static std::optional<Uuid> FromIntArrayTag(IntArrayTag const &tag) {
    auto const &values = tag.value();
    if (values.size() != 4) {
      return std::nullopt;
    }
    i32 f1 = values[0];
    i32 f2 = values[1];
    i32 f3 = values[2];
    i32 f4 = values[3];
    Uuid u;
    *(i32 *)u.fData = mcfile::I32FromBE(f1);
    *((i32 *)u.fData + 1) = mcfile::I32FromBE(f2);
    *((i32 *)u.fData + 2) = mcfile::I32FromBE(f3);
    *((i32 *)u.fData + 3) = mcfile::I32FromBE(f4);
    return u;
  }
};

struct UuidHasher {
  size_t operator()(Uuid const &k) const {
    size_t res = 17;
    for (int i = 0; i < 16; i++) {
      res = res * 31 + std::hash<u8>{}(k.fData[i]);
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
