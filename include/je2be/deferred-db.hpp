#pragma once

namespace j2b {

class DeferredDb {
public:
    DeferredDb() {
        using namespace std;
        namespace fs = mcfile::detail::filesystem;
        auto tmp = fs::temp_directory_path();
        auto p = file::CreateTempDir(tmp);
        if (p) {
            fTmp = *p;
            fValid = true;
        }
    }

    ~DeferredDb() {
        if (!fValid) {
            return;
        }
        file::RemoveAll(fTmp);
    }

    bool valid() const {
        return fValid;
    }

    void put(std::string const& key, leveldb::Slice const& value) {
        namespace fs = mcfile::detail::filesystem;
        using namespace std;
        assert(fValid);
        size_t index;
        size_t const kMaxValueSize = 1024;
        size_t const kMaxPutSmallCacheLength = 1024;
        bool small = value.size() <= kMaxValueSize;
        unordered_map<string, string> flushPutSmall;
        {
            std::lock_guard<std::mutex> lk(fMutex);
            if (small) {
                fPutSmall.insert(make_pair(key, value.ToString()));
                if (fPutSmall.size() > kMaxPutSmallCacheLength) {
                    flushPutSmall.swap(fPutSmall);
                } else {
                    return;
                }
            } else {
                index = fPut.size();
                fPut.push_back(key);
            }
        }
        if (small) {
            if (!flushPutSmall.empty()) {
                std::lock_guard<std::mutex> lk(fSmallMutex);
                FILE* f = file::Open(fTmp / "SMALL", "a+b");
                for (auto const& it : flushPutSmall) {
                    auto const& k = it.first;
                    auto const& v = it.second;
                    uint32_t s = k.size();
                    fwrite(&s, sizeof(s), 1, f);
                    fwrite(k.data(), k.size(), 1, f);
                    s = v.size();
                    fwrite(&s, sizeof(s), 1, f);
                    fwrite(v.data(), v.size(), 1, f);
                }
                fclose(f);
            }
        } else {
            auto p = fTmp / (to_string(index) + ".bin");
            FILE* f = file::Open(p, "wb");
            uint32_t size = value.size();
            fwrite(&size, sizeof(size), 1, f);
            fwrite(value.data(), size, 1, f);
            fclose(f);
        }
    }

    void del(std::string const& key) {
        std::lock_guard<std::mutex> lk(fMutex);
        fDel.push_back(key);
    }

    void flush(Db& db) {
        assert(fValid);
        using namespace std;
        namespace fs = mcfile::detail::filesystem;
        leveldb::WriteBatch batch;
        size_t kMaxBatchSize = 4086;
        for (size_t i = 0; i < fPut.size(); i++) {
            string key = fPut[i];
            auto p = fTmp / (to_string(i) + ".bin");
            FILE* f = file::Open(p, "rb");
            uint32_t size = 0;
            fread(&size, sizeof(size), 1, f);
            vector<char> buffer(size);
            fread(buffer.data(), size, 1, f);
            fclose(f);
            leveldb::Slice value(buffer.data(), size);
            batch.Put(key, value);
            if (batch.ApproximateSize() > kMaxBatchSize) {
                db.write(batch);
                batch.Clear();
            }
        }
        for (auto const& it : fPutSmall) {
            batch.Put(it.first, it.second);
            if (batch.ApproximateSize() > kMaxBatchSize) {
                db.write(batch);
                batch.Clear();
            }
        }
        {
            FILE* f = file::Open(fTmp / "SMALL", "rb");
            if (f) {
                vector<char> buffer;
                while (true) {
                    uint32_t s = 0;
                    if (fread(&s, sizeof(s), 1, f) != 1) {
                        break;
                    }
                    buffer.resize(s);
                    fread(buffer.data(), s, 1, f);
                    string key(buffer.data(), s);
                    s = 0;
                    if (fread(&s, sizeof(s), 1, f) != 1) {
                        break;
                    }
                    buffer.resize(s);
                    fread(buffer.data(), s, 1, f);
                    string value(buffer.data(), s);
                    batch.Put(key, value);
                    if (batch.ApproximateSize() > kMaxBatchSize) {
                        db.write(batch);
                        batch.Clear();
                    }
                }
                fclose(f);
            }
        }
        for (string key : fDel) {
            batch.Delete(key);
        }
        db.write(batch);
    }

private:
    std::filesystem::path fTmp;
    bool fValid = false;
    std::mutex fMutex;
    std::vector<std::string> fDel;
    std::vector<std::string> fPut;
    std::unordered_map<std::string, std::string> fPutSmall;
    std::mutex fSmallMutex;
};

}
