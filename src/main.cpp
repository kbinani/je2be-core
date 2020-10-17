#include <string>
#include <thread>

#include <je2be.hpp>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return -1;
    }
    std::string const input = argv[1];
    std::string const output = argv[2];

    j2e::Converter::InputOption io;
    io.fLevelDirectoryStructure = j2e::LevelDirectoryStructure::Paper;
    j2e::Converter::OutputOption oo;
    j2e::Converter converter(input, io, output, oo);
    return converter.run(std::thread::hardware_concurrency()) ? 0 : -1;
}
