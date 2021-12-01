#include <je2be.hpp>

#include <iostream>
#include <string>
#include <thread>

static void PrintHelpMessage() {
  std::cerr << "cli -i [INPUT:directory] -o [OUTPUT:directory] [-n [NUM_THREADS:number]] [-s [DIRECTORY_STRUCTURE:\"vanilla\"|\"paper\"]]" << std::endl;
}

int main(int argc, char *argv[]) {
  std::string input;
  std::string output;
  je2be::LevelDirectoryStructure structure = je2be::LevelDirectoryStructure::Vanilla;
  unsigned int concurrency = std::thread::hardware_concurrency();

  char opt = 0;
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg.empty()) {
      continue;
    }
    if (arg[0] == '-' && arg.size() > 1) {
      opt = arg[1];
      if (opt == 'h') {
        PrintHelpMessage();
        return 0;
      }
    } else {
      switch (opt) {
      case 'i':
        input = arg;
        break;
      case 'o':
        output = arg;
        break;
      case 's':
        if (arg == "vanilla") {
          structure = je2be::LevelDirectoryStructure::Vanilla;
        } else if (arg == "paper") {
          structure = je2be::LevelDirectoryStructure::Paper;
        } else {
          std::cerr << "error: unknown LevelDirectoryStructure name; should be \"vanilla\" or \"paper\"" << std::endl;
          return -1;
        }
        break;
      case 'n': {
        auto n = je2be::strings::Toi(arg);
        if (!n) {
          std::cerr << "error: -n option must be an integer" << std::endl;
          return -1;
        }
        if (*n < 1) {
          std::cerr << "error: -n option must be > 0" << std::endl;
          return -1;
        }
        concurrency = *n;
        break;
      }
      default:
        std::cerr << "error: unknown option" << std::endl;
        PrintHelpMessage();
        return -1;
      }
    }
  }

  if (input.empty()) {
    std::cerr << "error: no input directory" << std::endl;
    return -1;
  }
  if (output.empty()) {
    std::cerr << "error: no output directory" << std::endl;
    return -1;
  }

  je2be::tobe::InputOption io;
  io.fLevelDirectoryStructure = structure;
  je2be::tobe::OutputOption oo;
  je2be::tobe::Converter converter(std::filesystem::path(input), io,
                                   std::filesystem::path(output), oo);
  return converter.run(concurrency) ? 0 : -1;
}
