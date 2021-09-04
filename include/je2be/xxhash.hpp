#pragma once

namespace je2be {

class XXHash {
public:
  XXHash() : fState(XXH64_createState()) {
    XXH64_hash_t seed = 0;
    XXH64_reset(fState, seed);
  }

  void update(void const *p, size_t size) { XXH64_update(fState, p, size); }

  int64_t digest() const {
    XXH64_hash_t hash = XXH64_digest(fState);
    return *(int64_t *)&hash;
  }

  ~XXHash() { XXH64_freeState(fState); }

  static int64_t Digest(void const *p, size_t size) {
    XXHash h;
    h.update(p, size);
    return h.digest();
  }

private:
  XXHash(XXHash const &) = delete;
  XXHash &operator=(XXHash const &) = delete;

private:
  XXH64_state_t *fState = nullptr;
};

} // namespace je2be
