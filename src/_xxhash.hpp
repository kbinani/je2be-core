#pragma once

#include <xxhash64.h>

namespace je2be {

class XXHash {
public:
  XXHash() : fHasher(0) {
  }

  void update(void const *p, size_t size) {
    fHasher.add(p, size);
  }

  i64 digest() const {
    u64 h = fHasher.hash();
    return *(i64 *)&h;
  }

  static i64 Digest(void const *p, size_t size) {
    XXHash h;
    h.update(p, size);
    return h.digest();
  }

private:
  XXHash(XXHash const &) = delete;
  XXHash &operator=(XXHash const &) = delete;

private:
  XXHash64 fHasher;
};

} // namespace je2be
