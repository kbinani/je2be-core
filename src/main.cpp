#include <minecraft-file.hpp>
#include <leveldb/db.h>
#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
    using namespace std;

    if (argc != 3) {
        return -1;
    }
    string const input = argv[1];
    string const output = argv[2];

    leveldb::DB *db;
    leveldb::Options options;
    leveldb::Status status = leveldb::DB::Open(options, input.c_str(), &db);
    if (!status.ok()) {
        cerr << status.ToString() << endl;
        return -1;
    }

    //TODO:

    return 0;
}
