#include <cxxopts.hpp>
#include <je2be.hpp>

#include <iostream>
#include <thread>

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace je2be::toje;
  namespace fs = std::filesystem;

  cxxopts::Options parser("b2j");
  parser.add_options()                                    //
      ("i", "input directory", cxxopts::value<string>())  //
      ("o", "output directory", cxxopts::value<string>()) //
      ("n", "num threads", cxxopts::value<unsigned int>()->default_value(to_string(thread::hardware_concurrency())));
  cxxopts::ParseResult result;
  try {
    result = parser.parse(argc, argv);
  } catch (cxxopts::OptionException &e) {
    cerr << e.what() << endl;
    cerr << parser.help() << endl;
    return -1;
  }

  string inputString = result["i"].as<string>();
  fs::path input(inputString);
  if (!fs::is_directory(input)) {
    cerr << "error: input directory does not exist" << endl;
    return -1;
  }

  string outputString = result["o"].as<string>();
  if (outputString.empty()) {
    cerr << "error: output directory not set" << endl;
    return -1;
  }
  fs::path output(outputString);

  unsigned int concurrency = result["n"].as<unsigned int>();

  auto start = chrono::high_resolution_clock::now();
  defer {
    auto elapsed = chrono::high_resolution_clock::now() - start;
    cout << float(chrono::duration_cast<chrono::milliseconds>(elapsed).count() / 1000.0f) << "s" << endl;
  };

  Options options;
  options.fTempDirectory = mcfile::File::CreateTempDir(fs::temp_directory_path());
  defer {
    if (options.fTempDirectory) {
      je2be::Fs::DeleteAll(*options.fTempDirectory);
    }
  };
#if 0
  int s = 1;
  options.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  for (int x = -s; x <= s; x++) {
    for (int z = -s; z <= s; z++) {
      options.fChunkFilter.insert({x, z});
    }
  }
#endif
  auto st = Converter::Run(input, output, options, concurrency);
  return st.ok() ? 0 : -1;
}
