#pragma once

#include <hwm/task/task_queue.hpp>

#include <je2be/future-support.hpp>

namespace je2be {

class AsyncIterator {
  struct Result {
    char fPrefix;
    std::shared_ptr<leveldb::Iterator> fItr;
    bool fContinue;
    bool fValid;
    std::string fKey;
    std::string fValue;
  };

  AsyncIterator() = delete;

public:
  using Receiver = std::function<void(std::string const &key, std::string const &value)>;

  static void IterateUnordered(leveldb::DB &db, unsigned int concurrency, Receiver receiver) {
    using namespace std;
    using namespace leveldb;

    hwm::task_queue queue(concurrency);
    deque<future<Result>> futures;

    for (int i = 0; i < 256; i++) {
      shared_ptr<Iterator> itr(db.NewIterator({}));
      string prefix;
      prefix.append(1, (char)i);
      itr->Seek(Slice(prefix));
      futures.push_back(queue.enqueue(Do, (char)i, itr));
    }
    while (!futures.empty()) {
      vector<future<Result>> drain;
      FutureSupport::Drain(thread::hardware_concurrency() + 1, futures, drain);
      for (auto &f : drain) {
        Result ret = f.get();
        if (ret.fValid) {
          receiver(ret.fKey, ret.fValue);
        }
        if (ret.fContinue) {
          futures.push_back(queue.enqueue(Do, ret.fPrefix, ret.fItr));
        }
      }
    }
  }

private:
  static Result Do(char prefix, std::shared_ptr<leveldb::Iterator> itr) {
    Result ret;
    ret.fPrefix = prefix;
    ret.fItr = itr;

    if (itr->Valid()) {
      std::string key = itr->key().ToString();
      ret.fValid = prefix == key[0];
      if (prefix == key[0]) {
        ret.fContinue = true;
        ret.fKey = key;
        ret.fValue = itr->value().ToString();
        itr->Next();
      } else {
        ret.fContinue = false;
      }
    } else {
      ret.fContinue = false;
      ret.fValid = false;
    }
    return ret;
  }
};

} // namespace je2be
