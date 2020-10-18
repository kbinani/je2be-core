#include <string>
#include <thread>

#include <je2be.hpp>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return -1;
    }
    std::string const input = argv[1];
    std::string const output = argv[2];

    j2b::Converter::InputOption io;
    io.fLevelDirectoryStructure = j2b::LevelDirectoryStructure::Paper;
    j2b::Converter::OutputOption oo;
    j2b::Converter converter(input, io, output, oo);
    return converter.run(std::thread::hardware_concurrency()) ? 0 : -1;
}
