#pragma once

#include "_parallel.hpp"

namespace je2be {

class AsyncIterator {
  AsyncIterator() = delete;

public:
  template <class Accumulator>
  static std::pair<Accumulator, Status> IterateUnordered(
      leveldb::DB &db,
      unsigned int concurrency,
      Accumulator zero,
      std::function<void(std::string const &key, std::string const &value, Accumulator &out)> accept,
      std::function<void(Accumulator const &from, Accumulator &to)> join) {
    using namespace std;
    using namespace leveldb;

    vector<char> works(256);
    for (int i = 0; i < 256; i++) {
      works[i] = (char)i;
    }
    auto j = [join](Accumulator const &from, Accumulator &to) -> void {
      join(from, to);
    };
    return Parallel::Reduce<char, Accumulator>(
        works,
        concurrency,
        zero,
        [&db, zero, accept](char prefix) -> pair<Accumulator, Status> {
          ReadOptions o;
          o.fill_cache = false;
          shared_ptr<Iterator> itr(db.NewIterator(o));
          Accumulator sum = zero;
          if (auto st = itr->status(); !st.ok()) {
            return make_pair(sum, Status::FromLevelDBStatus(st));
          }
          string p;
          p.append(1, prefix);
          for (itr->Seek(Slice(p)); itr->Valid(); itr->Next()) {
            if (auto st = itr->status(); !st.ok()) {
              return make_pair(sum, Status::FromLevelDBStatus(st));
            }
            auto key = itr->key().ToString();
            if (key[0] != prefix) {
              break;
            }
            auto value = itr->value().ToString();
            accept(key, value, sum);
          }
          return make_pair(sum, Status::Ok());
        },
        j);
  }
};

} // namespace je2be
