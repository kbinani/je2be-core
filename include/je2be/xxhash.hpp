#pragma once

namespace je2be {

class XXHash {
public:
  XXHash() : fHasher(0) {
  }

  void update(void const *p, size_t size) {
    fHasher.add(p, size);
  }

  int64_t digest() const {
    uint64_t h = fHasher.hash();
    return *(int64_t *)&h;
  }

  static int64_t Digest(void const *p, size_t size) {
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
