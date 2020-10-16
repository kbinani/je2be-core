#include <string>
#include <iostream>

#include <je2be.hpp>

int main(int argc, char *argv[]) {
    using namespace std;
    namespace fs = mcfile::detail::filesystem;
    using namespace j2e;

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

    Db db(dbPath.string());
    if (!db.valid()) {
        return -1;
    }

    return 0;
}
