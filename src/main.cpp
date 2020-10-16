#include <minecraft-file.hpp>
#include <leveldb/db.h>
#include <table/compression/zlib_compressor.h>
#include <string>
#include <iostream>
#include <memory>
#include <sys/stat.h>

#include "props.hpp"
#include "version.hpp"
#include "level.hpp"

int main(int argc, char *argv[]) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    using namespace mcfile::nbt;
    namespace fs = mcfile::detail::filesystem;

    if (argc != 3) {
        return -1;
    }
    string const input = argv[1];
    string const output = argv[2];

    auto rootPath = fs::path(output);
    auto dbPath = rootPath / "db";

    fs::create_directory(rootPath);
    fs::create_directory(dbPath);
    Level level;
    level.write(output + string("/level.dat"));

    DB *db;
    Options options;
    options.compression = kZlibCompression;
    options.create_if_missing = true;
    Status status = DB::Open(options, dbPath.string().c_str(), &db);
    if (!status.ok()) {
        cerr << status.ToString() << endl;
        return -1;
    }

    delete db;

    return 0;
}
