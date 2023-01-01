#pragma once

#include <je2be/parallel.hpp>

namespace je2be {

class AsyncIterator {
  AsyncIterator() = delete;

public:
  template <class Accumulator>
  static Accumulator IterateUnordered(
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
    return Parallel::ForEach<Accumulator, char>(
        works,
        zero,
        [&db, zero, accept](char prefix) -> Accumulator {
          ReadOptions o;
          o.fill_cache = false;
          shared_ptr<Iterator> itr(db.NewIterator(o));
          Accumulator sum = zero;
          string p;
          p.append(1, prefix);
          for (itr->Seek(Slice(p)); itr->Valid(); itr->Next()) {
            auto key = itr->key().ToString();
            if (key[0] != prefix) {
              break;
            }
            auto value = itr->value().ToString();
            accept(key, value, sum);
          }
          return sum;
        },
        join);
  }
};

} // namespace je2be
