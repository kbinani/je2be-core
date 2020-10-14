#include <minecraft-file.hpp>
#include <leveldb/db.h>
#include <leveldb/zlib_compressor.h>
#include <leveldb/decompress_allocator.h>
#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
    using namespace std;
    using namespace leveldb;

    if (argc != 2) {
        return -1;
    }
    string const dir = argv[1];

    DB *db;
    Options options;
    options.compressors[0] = new ZlibCompressorRaw(-1);
    options.compressors[1] = new ZlibCompressor();
    Status status = DB::Open(options, dir.c_str(), &db);
    if (!status.ok()) {
        cerr << status.ToString() << endl;
        return -1;
    }

    cout << "DB::Open ok" << endl;

    ReadOptions ro;
    ro.decompress_allocator = new DecompressAllocator();
    
    leveldb::Iterator *it = db->NewIterator(ro);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        cout << it->key().ToString() << endl;
    }
    delete it;

    delete db;

    return 0;
}
