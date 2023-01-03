#pragma once

#include "db/_db-interface.hpp"

#include <iostream>

namespace je2be {

class ConcurrentDb : public DbInterface {
  class Writer {
  public:
    void put(std::string const &key, std::string const &value) {
      fWritten++;
    }
    int fWritten = 0;
  };

  class Gate {
  public:
    std::shared_ptr<Writer> get(uintptr_t key) {
      using namespace std;
      lock_guard<mutex> lock(Mut());
      auto found = fWriters.find(key);
      if (found != fWriters.end()) {
        return found->second;
      }
      auto writer = make_shared<Writer>();
      fWriters[key] = writer;
      Jar().insert(make_pair(key, writer));
      return writer;
    }

    static void Drain(uintptr_t key, std::vector<std::shared_ptr<Writer>> &out) {
      using namespace std;
      lock_guard<mutex> lock(Mut());
      auto &jar = Jar();
      while (true) {
        auto node = jar.extract(key);
        if (node.empty()) {
          break;
        }
        out.push_back(node.mapped());
      }
    }

    static std::mutex &Mut() {
      static std::mutex sMut;
      return sMut;
    }

    static std::multimap<uintptr_t, std::shared_ptr<Writer>> &Jar() {
      static std::multimap<uintptr_t, std::shared_ptr<Writer>> sJar;
      return sJar;
    }

    void remove(uintptr_t key) {
      using namespace std;
      lock_guard<mutex> lock(Mut());
      fWriters.erase(key);
    }

  private:
    std::map<uintptr_t, std::shared_ptr<Writer>> fWriters;
  };

public:
  explicit ConcurrentDb(std::filesystem::path const &dbname) : fDbName(dbname) {}

  ~ConcurrentDb() {
    gate().remove((uintptr_t)this);
  }

  bool valid() const override {
    return true;
  }

  void put(std::string const &key, leveldb::Slice const &value) override {
    gate().get((uintptr_t)this)->put(key, value.ToString());
  }

  void del(std::string const &key) override {}

  bool close(std::function<void(double progress)> progress = nullptr) override {
    using namespace std;
    vector<shared_ptr<Writer>> writers;
    Gate::Drain((uintptr_t)this, writers);
    cout << "writers.size()=" << writers.size() << endl;
    int total = 0;
    for (int i = 0; i < writers.size(); i++) {
      total += writers[i]->fWritten;
      cout << "#" << i << "; " << writers[i]->fWritten << endl;
    }
    cout << "total: " << total << endl;
    return true;
  }

  void abandon() override {}

private:
  Gate &gate() {
    using namespace std;
    thread_local unique_ptr<Gate> tGate(new Gate);
    return *tGate;
  }

private:
  std::filesystem::path const fDbName;
};

} // namespace je2be
