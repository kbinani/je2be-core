#pragma once

namespace j2b {

class ChunkData {
public:
  ChunkData(int32_t chunkX, int32_t chunkZ, Dimension dim)
      : fChunkX(chunkX), fChunkZ(chunkZ), fDimension(dim) {}

  void put(DbInterface &db) {
    auto status = putChunkSections(db);
    if (status == ChunkStatus::NotEmpty) {
      putVersion(db);
      putData2D(db);
      putBlockEntity(db);
      putEntity(db);
      putChecksums(db);
      putFinalizedState(db);
    }
  }

private:
  void putFinalizedState(DbInterface &db) const {
    auto key = Key::FinalizedState(fChunkX, fChunkZ, fDimension);
    if (fFinalizedState) {
      int32_t v = *fFinalizedState;
      db.put(key, leveldb::Slice((char const *)&v, sizeof(v)));
    } else {
      db.del(key);
    }
  }

  void putEntity(DbInterface &db) const {
    auto key = Key::Entity(fChunkX, fChunkZ, fDimension);
    if (fEntity.empty()) {
      db.del(key);
    } else {
      leveldb::Slice d((char *)fEntity.data(), fEntity.size());
      db.put(key, d);
    }
  }

  void putChecksums(DbInterface &db) const {
    auto sum = checksums();
    auto checksumKey = Key::Checksums(fChunkX, fChunkZ, fDimension);
    db.put(checksumKey, sum);
  }

  void putBlockEntity(DbInterface &db) const {
    auto key = Key::BlockEntity(fChunkX, fChunkZ, fDimension);
    if (fBlockEntity.empty()) {
      db.del(key);
    } else {
      leveldb::Slice blockEntity((char *)fBlockEntity.data(),
                                 fBlockEntity.size());
      db.put(key, blockEntity);
    }
  }

  void putData2D(DbInterface &db) const {
    auto data2DKey = Key::Data2D(fChunkX, fChunkZ, fDimension);
    if (fData2D.empty()) {
      db.del(data2DKey);
    } else {
      leveldb::Slice data2D((char *)fData2D.data(), fData2D.size());
      db.put(data2DKey, data2D);
    }
  }

  enum class ChunkStatus {
    Empty,
    NotEmpty,
  };

  ChunkStatus putChunkSections(DbInterface &db) const {
    bool empty = true;
    for (int i = 0; i < 16; i++) {
      auto key = Key::SubChunk(fChunkX, i, fChunkZ, fDimension);
      if (fSubChunks[i].empty()) {
        db.del(key);
      } else {
        leveldb::Slice subchunk((char *)fSubChunks[i].data(),
                                fSubChunks[i].size());
        db.put(key, subchunk);
        empty = false;
      }
    }
    if (empty) {
      if (fFinalizedState && *fFinalizedState == 2) {
        return ChunkStatus::NotEmpty;
      } else {
        return ChunkStatus::Empty;
      }
    } else {
      return ChunkStatus::NotEmpty;
    }
  }

  void putVersion(DbInterface &db) const {
    char const kSubChunkVersion = 19;

    auto const &versionKey = Key::Version(fChunkX, fChunkZ, fDimension);
    leveldb::Slice version(&kSubChunkVersion, 1);
    db.put(versionKey, version);
  }

  std::string checksums() const {
    using namespace std;
    using namespace mcfile::stream;
    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
    w.write((uint32_t)0);

    // SubChunk
    uint32_t count = 0;
    for (int i = 0; i < 16; i++) {
      if (fSubChunks[i].empty()) {
        continue;
      }
      w.write((uint8_t)0x2f);
      w.write((uint8_t)i);
      uint64_t hash = GetXXHSum(fSubChunks[i]);
      w.write(hash);
      count++;
    }

    // Data2D
    if (!fData2D.empty()) {
      w.write((uint8_t)0x2d);
      w.write((uint8_t)0);
      uint64_t hash = GetXXHSum(fData2D);
      w.write(hash);
      count++;
    }

    // BlockEntity
    if (!fBlockEntity.empty()) {
      w.write((uint8_t)0x31);
      w.write((uint8_t)0);
      uint64_t hash = GetXXHSum(fBlockEntity);
      w.write(hash);
      count++;
    }

    // Entity
    if (!fEntity.empty()) {
      w.write((uint8_t)0x32);
      w.write((uint8_t)0);
      uint64_t hash = GetXXHSum(fEntity);
      w.write(hash);
      count++;
    }

    s->seek(0);
    w.write(count);

    std::vector<uint8_t> buffer;
    s->drain(buffer);
    string r((char *)buffer.data(), buffer.size());
    return r;
  }

  static uint64_t GetXXHSum(std::vector<uint8_t> const &v) {
    int64_t d = XXHash::Digest(v.data(), v.size());
    return *(uint64_t *)&d;
  }

public:
  int32_t const fChunkX;
  int32_t const fChunkZ;
  Dimension const fDimension;

  std::vector<uint8_t> fData2D;
  std::vector<uint8_t> fSubChunks[16];
  std::vector<uint8_t> fBlockEntity;
  std::vector<uint8_t> fEntity;
  std::optional<int32_t> fFinalizedState;
};

} // namespace j2b
