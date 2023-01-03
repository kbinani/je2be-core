#pragma once

#include <je2be/defer.hpp>
#include <je2be/uuid.hpp>

#include "_fs.hpp"
#include "_parallel.hpp"
#include "db/_db-interface.hpp"

#include <db/log_writer.h>
#include <db/version_edit.h>
#include <leveldb/env.h>
#include <table/block_builder.h>
#include <table/format.h>
#include <util/crc32c.h>

namespace je2be {

class ConcurrentDb : public DbInterface {
  class Writer {
  public:
    Writer(std::filesystem::path const &dbname, std::atomic_uint64_t &sequencer) : fDbName(dbname), fSequencer(sequencer) {
      namespace fs = std::filesystem;
      Uuid id = Uuid::Gen();
      fs::path dir = dbname / id.toString();
      if (!Fs::CreateDirectories(dir)) {
        return;
      }
      fDir = dir;

      fs::path keyFile = dir / "key.bin";
      FILE *key = mcfile::File::Open(keyFile, mcfile::File::Mode::Write);
      if (!key) {
        return;
      }

      fs::path valueFile = dir / "value.bin";
      FILE *value = mcfile::File::Open(valueFile, mcfile::File::Mode::Write);
      if (!value) {
        fclose(key);
        return;
      }

      fKey = key;
      fValue = value;
    }

    ~Writer() {
      abandon();
    }

    void put(std::string const &key, std::string const &value) {
      using namespace std;
      if (!fValue || !fKey) {
        return;
      }
      uint64_t sequence = fSequencer.fetch_add(1);
      string block;
      Compress(key, value, sequence, &block);

      uint32_t valueSizeCompressed = block.size();
      uint32_t keySize = key.size();

      if (fwrite(block.data(), block.size(), 1, fValue) != 1) {
        goto error;
      }

      if (fwrite(&keySize, sizeof(keySize), 1, fKey) != 1) {
        goto error;
      }
      if (fwrite(&valueSizeCompressed, sizeof(valueSizeCompressed), 1, fKey) != 1) {
        goto error;
      }
      if (fwrite(&fOffset, sizeof(fOffset), 1, fKey) != 1) {
        goto error;
      }
      if (fwrite(key.data(), key.size(), 1, fKey) != 1) {
        goto error;
      }
      fOffset += block.size();
      return;

    error:
      if (fKey) {
        fclose(fKey);
        fKey = nullptr;
      }
      if (fValue) {
        fclose(fValue);
        fValue = nullptr;
      }
      Fs::DeleteAll(fDir);
    }

    bool close(std::atomic_uint64_t &fileNumber) {
      if (!fValue || !fKey) {
        return false;
      }
      fclose(fValue);
      fValue = nullptr;
      fclose(fKey);
      fKey = nullptr;
      // TODO:
      return true;
    }

    void abandon() {
      if (fValue) {
        fclose(fValue);
        fValue = nullptr;
      }
      if (fKey) {
        fclose(fKey);
        fKey = nullptr;
      }
      Fs::DeleteAll(fDir);
    }

  private:
    static void Compress(std::string const &key, leveldb::Slice const &value, uint64_t seq, std::string *out) {
      using namespace std;
      using namespace leveldb;

      leveldb::Options o;
      o.compression = kZlibRawCompression;
      InternalKeyComparator icmp(BytewiseComparator());
      o.comparator = &icmp;

      leveldb::BlockBuilder bb(&o);
      InternalKey ik(key, seq, kTypeValue);
      bb.Add(ik.Encode(), value);
      Slice c = bb.Finish();

      mcfile::Compression::CompressDeflate((void *)c.data(), c.size(), *out);
    }

  private:
    std::filesystem::path fDbName;
    std::atomic_uint64_t &fSequencer;
    std::filesystem::path fDir;
    FILE *fKey = nullptr;
    FILE *fValue = nullptr;
    uint64_t fOffset = 0;
  };

  class Gate {
  public:
    std::shared_ptr<Writer> get(uintptr_t key, std::filesystem::path const &dbname, std::atomic_uint64_t &sequence) {
      using namespace std;
      lock_guard<mutex> lock(Mut());
      auto found = fWriters.find(key);
      if (found != fWriters.end()) {
        return found->second;
      }
      auto writer = make_shared<Writer>(dbname, sequence);
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

  private:
    std::map<uintptr_t, std::shared_ptr<Writer>> fWriters;
  };

public:
  explicit ConcurrentDb(std::filesystem::path const &dbname) : fDbName(dbname) {}

  ~ConcurrentDb() {
    abandon();
  }

  bool valid() const override {
    return fValid;
  }

  void put(std::string const &key, leveldb::Slice const &value) override {
    gate().get((uintptr_t)this, fDbName, fSequence)->put(key, value.ToString());
  }

  void del(std::string const &key) override {}

  bool close(std::function<void(double progress)> progress = nullptr) override {
    using namespace std;
    namespace fs = std::filesystem;
    vector<shared_ptr<Writer>> writers;
    Gate::Drain((uintptr_t)this, writers);
    atomic_uint64_t fileNumber(1);
    fValid = false;
    return Parallel::Reduce<shared_ptr<Writer>, bool>(
        writers,
        true,
        [&fileNumber](shared_ptr<Writer> const &writer) -> bool {
          return writer->close(fileNumber);
        },
        Parallel::Merge);
  }

  void abandon() override {
    using namespace std;
    namespace fs = std::filesystem;
    vector<shared_ptr<Writer>> writers;
    Gate::Drain((uintptr_t)this, writers);
    vector<fs::path> files;
    for (int i = 0; i < writers.size(); i++) {
      writers[i]->abandon();
    }
    fValid = false;
  }

private:
  Gate &gate() {
    using namespace std;
    thread_local unique_ptr<Gate> tGate(new Gate);
    return *tGate;
  }

private:
  std::filesystem::path const fDbName;
  std::atomic_uint64_t fSequence;
  bool fValid = true;
};

} // namespace je2be
