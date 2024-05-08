#pragma once

#include <je2be/uuid.hpp>

#include <mutex>

namespace je2be::java {

class UuidRegistrar {
public:
  i64 toId(Uuid const &uuid) {
    std::lock_guard<std::mutex> lock(fMut);
    auto found = fLut.find(uuid);
    if (found != fLut.end()) {
      return found->second;
    }
    i64 candidate = unsafeFirstCandidate(uuid);
    i64 result = unsafeAvoidCollision(candidate);
    fLut[uuid] = result;
    return result;
  }

  i64 leasherIdFor(i64 id) {
    std::lock_guard<std::mutex> lock(fMut);
    auto found = fLeasherLut.find(id);
    if (found != fLeasherLut.end()) {
      return found->second;
    }
    i64 candidate = mcfile::XXHash<i64>::Digest(&id, sizeof(id));
    i64 result = unsafeAvoidCollision(candidate);
    fLeasherLut[id] = result;
    return result;
  }

  i64 randomEntityId() {
    std::random_device rd;
    std::mt19937_64 mt(rd());
    std::uniform_int_distribution<i64> distribution(std::numeric_limits<i64>::lowest(), std::numeric_limits<i64>::max());
    i64 candidate = distribution(mt);
    std::lock_guard<std::mutex> lock(fMut);
    return unsafeAvoidCollision(candidate);
  }

private:
  i64 unsafeAvoidCollision(i64 h) {
    static std::unordered_set<i64> sUsed;

    while (sUsed.count(h) > 0) {
      h = mcfile::XXHash<i64>::Digest(&h, sizeof(h));
    }
    sUsed.insert(h);
    return h;
  }

  i64 unsafeFirstCandidate(Uuid const &uuid) {
    mcfile::XXHash<i64> h;
    h.update(uuid.fData, sizeof(uuid.fData));
    return h.digest();
  }

private:
  std::mutex fMut;
  std::unordered_map<i64, i64> fLeasherLut;
  std::unordered_map<Uuid, i64, UuidHasher, UuidPred> fLut;
};

} // namespace je2be::java
