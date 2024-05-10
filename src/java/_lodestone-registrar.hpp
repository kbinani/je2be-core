#pragma once

#include "_dimension-ext.hpp"
#include "db/_db-interface.hpp"

#include <mutex>

namespace je2be::java {

class LodestoneRegistrar {
public:
  i32 get(mcfile::Dimension d, Pos3i pos) {
    using namespace std;
    lock_guard<mutex> lock(fMut);
    auto key = make_tuple(d, pos.fX, pos.fY, pos.fZ);
    if (auto found = fTrackers.find(key); found != fTrackers.end()) {
      return found->second;
    }
    i32 tracker = fNext;
    fNext++;
    fTrackers[key] = tracker;
    return tracker;
  }

  Status put(DbInterface &db) {
    std::optional<i32> last;
    for (auto const &it : fTrackers) {
      auto [dim, x, y, z] = it.first;
      i32 tracker = it.second;
      if (last) {
        last = std::max(*last, tracker);
      } else {
        last = tracker;
      }
      auto c = Compound();
      i32 dimB = 0;
      switch (dim) {
      case mcfile::Dimension::Overworld:
        dimB = 0;
        break;
      case mcfile::Dimension::Nether:
        dimB = 1;
        break;
      case mcfile::Dimension::End:
        dimB = 2;
        break;
      }
      c->set(u8"dim", Int(dimB));
      auto id = mcfile::be::DbKey::TrackId(tracker);
      std::u8string u8id;
      std::copy(id.begin(), id.end(), std::back_inserter(u8id));
      c->set(u8"id", u8id);
      c->set(u8"pos", ListFromPos3i(Pos3i(x, y, z)));
      c->set(u8"status", Byte(0));
      c->set(u8"version", Byte(1));

      if (auto st = Put(db, mcfile::be::DbKey::PosTrackDB(tracker), *c); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    }
    if (last) {
      auto c = Compound();
      auto id = mcfile::be::DbKey::TrackId(*last);
      std::u8string u8id;
      std::copy(id.begin(), id.end(), std::back_inserter(u8id));
      c->set(u8"id", u8id);
      c->set(u8"version", Byte(1));
      return Put(db, mcfile::be::DbKey::PositionTrackDBLastId(), *c);
    } else {
      return Status::Ok();
    }
  }

private:
  static Status Put(DbInterface &db, std::string const &key, CompoundTag const &tag) {
    auto buffer = CompoundTag::Write(tag, mcfile::Encoding::LittleEndian);
    if (!buffer) {
      return JE2BE_ERROR;
    }

    leveldb::Slice v(*buffer);
    if (auto st = db.put(key, v); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    return Status::Ok();
  }

private:
  std::mutex fMut;
  i32 fNext = 1;
  std::map<std::tuple<mcfile::Dimension, i32, i32, i32>, i32> fTrackers;
};

} // namespace je2be::java
