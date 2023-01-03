#pragma once

#include <je2be/uuid.hpp>

#include <je2be/_xxhash.hpp>

#include <mutex>

namespace je2be::tobe {

class UuidRegistrar {
public:
  static int64_t ToId(Uuid const &uuid) {
    static std::unordered_map<Uuid, int64_t, UuidHasher, UuidPred> sLut;

    std::mutex *mut = Mut();
    std::lock_guard<std::mutex> lock(*mut);
    auto found = sLut.find(uuid);
    if (found != sLut.end()) {
      return found->second;
    }
    int64_t candidate = FirstCandidate(uuid);
    int64_t result = AvoidCollision(candidate);
    sLut[uuid] = result;
    return result;
  }

  static int64_t LeasherIdFor(int64_t id) {
    static std::unordered_map<int64_t, int64_t> sLut;

    std::mutex *mut = Mut();
    std::lock_guard<std::mutex> lock(*mut);
    auto found = sLut.find(id);
    if (found != sLut.end()) {
      return found->second;
    }
    int64_t candidate = XXHash::Digest(&id, sizeof(id));
    int64_t result = AvoidCollision(candidate);
    sLut[id] = result;
    return result;
  }

private:
  static std::mutex *Mut() {
    static std::mutex sMut;
    return &sMut;
  }

  static int64_t AvoidCollision(int64_t h) {
    static std::unordered_set<int64_t> sUsed;

    while (true) {
      if (sUsed.find(h) == sUsed.end()) [[likely]] {
        sUsed.insert(h);
        return h;
      }
      h = XXHash::Digest(&h, sizeof(h));
    }
  }

  static int64_t FirstCandidate(Uuid const &uuid) {
    XXHash h;
    h.update(uuid.fData, sizeof(uuid.fData));
    return h.digest();
  }

private:
  UuidRegistrar() = delete;
};

} // namespace je2be::tobe
