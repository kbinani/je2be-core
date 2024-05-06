#pragma once

#include <je2be/uuid.hpp>

#include <mutex>

namespace je2be::java {

class UuidRegistrar {
public:
  static i64 ToId(Uuid const &uuid) {
    static std::unordered_map<Uuid, i64, UuidHasher, UuidPred> sLut;

    std::mutex *mut = Mut();
    std::lock_guard<std::mutex> lock(*mut);
    auto found = sLut.find(uuid);
    if (found != sLut.end()) {
      return found->second;
    }
    i64 candidate = FirstCandidate(uuid);
    i64 result = AvoidCollision(candidate);
    sLut[uuid] = result;
    return result;
  }

  static i64 LeasherIdFor(i64 id) {
    static std::unordered_map<i64, i64> sLut;

    std::mutex *mut = Mut();
    std::lock_guard<std::mutex> lock(*mut);
    auto found = sLut.find(id);
    if (found != sLut.end()) {
      return found->second;
    }
    i64 candidate = mcfile::XXHash<i64>::Digest(&id, sizeof(id));
    i64 result = AvoidCollision(candidate);
    sLut[id] = result;
    return result;
  }

  static i64 RandomEntityId() {
    std::random_device rd;
    std::mt19937_64 mt(rd());
    std::uniform_int_distribution<i64> distribution(std::numeric_limits<i64>::lowest(), std::numeric_limits<i64>::max());
    i64 candidate = distribution(mt);
    return AvoidCollision(candidate);
  }

private:
  static std::mutex *Mut() {
    static std::mutex sMut;
    return &sMut;
  }

  static i64 AvoidCollision(i64 h) {
    static std::unordered_set<i64> sUsed;

    while (sUsed.count(h) > 0) {
      h = mcfile::XXHash<i64>::Digest(&h, sizeof(h));
    }
    sUsed.insert(h);
    return h;
  }

  static i64 FirstCandidate(Uuid const &uuid) {
    mcfile::XXHash<i64> h;
    h.update(uuid.fData, sizeof(uuid.fData));
    return h.digest();
  }

private:
  UuidRegistrar() = delete;
};

} // namespace je2be::java
