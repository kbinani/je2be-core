#pragma once

namespace j2b {

class ChunkData {
public:
    ChunkData(int32_t chunkX, int32_t chunkZ, Dimension dim)
        : fChunkX(chunkX)
        , fChunkZ(chunkZ)
        , fDimension(dim)
    {}

    void put(DbInterface& db) {
        if (putChunkSections(db)) {
            return;
        }
        putVersion(db);
        putData2D(db);
        putBlockEntity(db);
        putChecksums(db);
    }

private:
    void putChecksums(DbInterface& db) {
        auto sum = checksums();
        auto checksumKey = Key::Checksums(fChunkX, fChunkZ, fDimension);
        db.put(checksumKey, sum);
    }

    void putBlockEntity(DbInterface& db) const {
        auto key = Key::BlockEntity(fChunkX, fChunkZ, fDimension);
        if (fBlockEntity.empty()) {
            db.del(key);
        } else {
            leveldb::Slice blockEntity((char*)fBlockEntity.data(), fBlockEntity.size());
            db.put(key, blockEntity);
        }
    }

    void putData2D(DbInterface& db) const {
        leveldb::Slice data2D((char*)fData2D.data(), fData2D.size());
        auto data2DKey = Key::Data2D(fChunkX, fChunkZ, fDimension);
        db.put(data2DKey, data2D);
    }

    bool putChunkSections(DbInterface& db) const {
        bool empty = true;
        for (int i = 0; i < 16; i++) {
            auto key = Key::SubChunk(fChunkX, i, fChunkZ, fDimension);
            if (fSubChunks[i].empty()) {
                db.del(key);
            } else {
                leveldb::Slice subchunk((char*)fSubChunks[i].data(), fSubChunks[i].size());
                db.put(key, subchunk);
                empty = false;
            }
        }
        return empty;
    }

    void putVersion(DbInterface& db) const {
        char const kSubChunkVersion = 19;

        auto const& versionKey = Key::Version(fChunkX, fChunkZ, fDimension);
        leveldb::Slice version(&kSubChunkVersion, 1);
        db.put(versionKey, version);
    }

    std::string checksums() const {
        using namespace std;
        using namespace mcfile::stream;
        auto s = make_shared<ByteStream>();
        OutputStreamWriter w(s, { .fLittleEndian = true });
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
        {
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

        s->seek(0);
        w.write(count);

        std::vector<uint8_t> buffer;
        s->drain(buffer);
        string r((char*)buffer.data(), buffer.size());
        return r;
    }

    static XXH64_hash_t GetXXHSum(std::vector<uint8_t> const& v) {
        XXH64_state_t* state = XXH64_createState();
        XXH64_hash_t seed = 0;
        XXH64_reset(state, seed);
        XXH64_update(state, v.data(), v.size());
        XXH64_hash_t hash = XXH64_digest(state);
        XXH64_freeState(state);
        return hash;
    }

public:
    int32_t const fChunkX;
    int32_t const fChunkZ;
    Dimension const fDimension;

    std::vector<uint8_t> fData2D;
    std::vector<uint8_t> fSubChunks[16];
    std::vector<uint8_t> fBlockEntity;
};

}
