#pragma once

namespace j2b {

class RawDb : public DbInterface {
public:
    RawDb(std::string const& dir, unsigned int concurrency) : fValid(true), fDir(dir) {
        using namespace std;
        using namespace leveldb;
        namespace fs = std::filesystem;

        fStop.store(false);
        thread th([this, concurrency]() {
            for (auto const& e : fs::directory_iterator(fs::path(fDir))) {
                fs::remove_all(e.path());
            }

            Options o;
            o.compression = kZlibRawCompression;

            uint32_t fileNum = 1;
            uint64_t sequenceNumber = 1;
            uint64_t const kMaxFileSize = 2 * 1024 * 1024;
            auto batch = make_shared<WriteBatch>();

            ThreadPool pool(concurrency);
            pool.init();

            deque<future<void>> futures;

            while (true) {
                unique_lock<std::mutex> lk(fMut);
                fCv.wait(lk, [this] { return !fQueue.empty() || fStop.load(); });
                vector<pair<string, string>> commands;
                fQueue.swap(commands);
                bool const stop = fStop.load();
                lk.unlock();
                for (auto const& c : commands) {
                    auto const& key = c.first;
                    auto const& value = c.second;
                    InternalKey ik(key, sequenceNumber, kTypeValue);
                    sequenceNumber++;
                    Slice k = ik.Encode();
                    batch->Put(k, value);
                    if (batch->ApproximateSize() > o.max_file_size) {
                        if (futures.size() > 10 * concurrency) {
                            for (unsigned int i = 0; i < 5 * concurrency; i++) {
                                futures.front().get();
                                futures.pop_front();
                            }
                        }

                        futures.emplace_back(move(pool.submit([this, fileNum, o](shared_ptr<WriteBatch> b) {
                            drainWriteBatch(*b, fileNum, o);
                        }, batch)));

                        fileNum++;
                        batch = make_shared<WriteBatch>();
                    }
                }
                if (stop) {
                    break;
                }
            }

            for (auto& f : futures) {
                f.get();
            }

            pool.shutdown();

            drainWriteBatch(*batch, fileNum, o);

            RepairDB(fDir.c_str(), o);
            DB* db = nullptr;
            Status st = DB::Open(o, fDir, &db);
            if (st.ok()) {
                delete db;
            }
        });
        fTh.swap(th);
    }

    bool valid() const override {
        return fValid;
    }

    void put(std::string const& key, leveldb::Slice const& value) override {
        auto cmd = std::make_pair(key, value.ToString());
        {
            std::lock_guard<std::mutex> lk(fMut);
            fQueue.push_back(cmd);
        }
        fCv.notify_one();
    }

    void del(std::string const& key) override {}

    ~RawDb() {
        fStop.store(true);
        fCv.notify_one();
        fTh.join();
    }

private:
    void drainWriteBatch(leveldb::WriteBatch& b, uint32_t fileNum, leveldb::Options o) {
        using namespace leveldb;
        using namespace std;
        unique_ptr<WritableFile> file(open(fileNum));
        unique_ptr<TableBuilder> tb(new TableBuilder(o, file.get()));
        struct Visitor : public WriteBatch::Handler {
            void Put(const Slice& key, const Slice& value) override {
                fDrain.insert(make_pair(key.ToString(), value.ToString()));
                fKeys.push_back(key.ToString());
            }
            void Delete(const Slice& key) override {}

            std::unordered_map<std::string, std::string> fDrain;
            std::vector<std::string> fKeys;
        } v;
        b.Iterate(&v);
        sort(v.fKeys.begin(), v.fKeys.end(), [](string const& lhs, string const& rhs) {
            return BytewiseComparator()->Compare(lhs, rhs) < 0;
        });
        for (auto const& k : v.fKeys) {
            tb->Add(k, v.fDrain[k]);
        }
        tb->Finish();
        file->Close();
        b.Clear();
    }

    std::string tableFile(uint32_t tableNumber) const {
        std::vector<char> buffer(11, (char)0);
        sprintf(buffer.data(), "%06d.ldb", tableNumber);
        std::string p(buffer.data(), 10);
        return fDir + "/" + p;
    }

    leveldb::WritableFile *open(uint32_t tableNumber) const {
        using namespace leveldb;
        Env *env = Env::Default();
        WritableFile *f = nullptr;
        auto fname = tableFile(tableNumber);
        Status s = env->NewWritableFile(fname.c_str(), &f);
        if (!s.ok()) {
            return nullptr;
        }
        return f;
    }

private:
    std::thread fTh;
    std::mutex fMut;
    bool fReady = false;
    std::vector<std::pair<std::string, std::string>> fQueue;
    std::condition_variable fCv;
    std::atomic_bool fStop;
    bool fValid = false;
    std::string const fDir;
};

}
