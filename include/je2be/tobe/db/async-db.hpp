#pragma once

namespace je2be::tobe {

class AsyncDb : public DbInterface {
private:
  enum class Op {
    PUT,
    DEL,
  };
  class Command {
  public:
    static Command Put(std::string const &key, std::string const &value) {
      Command c;
      c.fOp = Op::PUT;
      c.fKey = key;
      c.fValue = value;
      return c;
    }

    static Command Del(std::string const &key) {
      Command c;
      c.fOp = Op::DEL;
      c.fKey = key;
      return c;
    }

  public:
    Op fOp;
    std::string fKey;
    std::optional<std::string> fValue;
  };

public:
  AsyncDb(std::string const &dir) = delete;
  AsyncDb(std::wstring const &dir) = delete;
  explicit AsyncDb(std::filesystem::path const &dir) {
    fStop.store(false);
    Db *db = new Db(dir);
    fValid = db->valid();
    std::thread th([this, dir, db]() {
      while (true) {
        std::unique_lock<std::mutex> lk(fMut);
        fCv.wait(lk, [this] { return !fQueue.empty() || fStop.load(); });
        std::vector<Command> commands;
        fQueue.swap(commands);
        bool const stop = fStop.load();
        lk.unlock();
        for (auto const &c : commands) {
          switch (c.fOp) {
          case Op::PUT:
            db->put(c.fKey, *c.fValue);
            break;
          case Op::DEL:
            db->del(c.fKey);
            break;
          }
        }
        if (stop) {
          break;
        }
      }
      delete db;
    });
    fTh.swap(th);
  }

  bool valid() const override { return fValid; }

  void put(std::string const &key, leveldb::Slice const &value) override {
    Command cmd = Command::Put(key, value.ToString());
    {
      std::lock_guard<std::mutex> lk(fMut);
      fQueue.push_back(cmd);
    }
    fCv.notify_one();
  }

  void del(std::string const &key) override {
    Command cmd = Command::Del(key);
    {
      std::lock_guard<std::mutex> lk(fMut);
      fQueue.push_back(cmd);
    }
    fCv.notify_one();
  }

  ~AsyncDb() {
    fStop.store(true);
    fCv.notify_one();
    fTh.join();
  }

private:
  std::thread fTh;
  std::mutex fMut;
  bool fReady = false;
  std::vector<Command> fQueue;
  std::condition_variable fCv;
  std::atomic_bool fStop;
  bool fValid = false;
};

} // namespace je2be::tobe
