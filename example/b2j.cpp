#include <je2be.hpp>

#include <iostream>

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  string input;
  string output;
  int concurrency = thread::hardware_concurrency();

  char opt = 0;
  for (int i = 1; i < argc; i++) {
    string arg = argv[i];
    if (arg.empty()) {
      continue;
    }
    if (arg[0] == '-' && arg.size() > 1) {
      opt = arg[1];
    } else {
      switch (opt) {
      case 'i':
        input = arg;
        break;
      case 'o':
        output = arg;
        break;
      case 'n': {
        auto n = je2be::strings::Toi(arg);
        if (!n) {
          cerr << "error: -n option must be an integer" << endl;
          return -1;
        }
        concurrency = *n;
        break;
      }
      default:
        cerr << "error: unknown option" << endl;
        return -1;
      }
    }
  }

  if (input.empty()) {
    cerr << "error: no input directory" << endl;
    return -1;
  }
  if (output.empty()) {
    cerr << "error: no output directory" << endl;
    return -1;
  }

  using namespace je2be::toje;

  fs::path i(input);
  fs::path o(output);
  Converter converter(i, o);
  return converter.run(concurrency) ? 0 : -1;
}