#pragma once

#include <BS_thread_pool.hpp>

#include <je2be/future-support.hpp>

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

    BS::thread_pool queue(concurrency);
    deque<future<Accumulator>> futures;

    Accumulator sum = zero;

    for (int i = 0; i < 256; i++) {
      vector<future<Accumulator>> drain;
      FutureSupport::Drain(concurrency + 1, futures, drain);
      for (auto &f : drain) {
        Accumulator r(f.get());
        join(r, sum);
      }
      futures.push_back(queue.submit(Do<Accumulator>, &db, (char)i, zero, accept));
    }
    for (auto &f : futures) {
      Accumulator r(f.get());
      join(r, sum);
    }
    return sum;
  }

private:
  template <class Accumulator>
  static Accumulator Do(
      leveldb::DB *db,
      char prefix, Accumulator zero,
      std::function<void(std::string const &key, std::string const &value, Accumulator &out)> accept) {
    using namespace std;
    using namespace leveldb;
    ReadOptions o;
    o.fill_cache = false;
    shared_ptr<Iterator> itr(db->NewIterator(o));
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
  }
};

} // namespace je2be
