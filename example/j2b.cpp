#include <cxxopts.hpp>
#include <je2be.hpp>

#if __has_include(<mimalloc.h>)
#include <mimalloc.h>
#endif

#include <iostream>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace je2be;
  using namespace je2be::tobe;
  namespace fs = std::filesystem;

#if __has_include(<mimalloc.h>)
  mi_version();
#endif

  cxxopts::Options parser("j2b");
  parser.add_options()                                                                                               //
      ("i", "input directory", cxxopts::value<string>())                                                             //
      ("o", "output directory", cxxopts::value<string>())                                                            //
      ("n", "num threads", cxxopts::value<unsigned int>()->default_value(to_string(thread::hardware_concurrency()))) //
      ("s", "directory structure", cxxopts::value<string>()->default_value("vanilla"));
  cxxopts::ParseResult result;
  try {
    result = parser.parse(argc, argv);
  } catch (cxxopts::exceptions::exception &e) {
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

  je2be::LevelDirectoryStructure structure = je2be::LevelDirectoryStructure::Vanilla;
  string structureString = result["s"].as<string>();
  if (structureString == "paper") {
    structure = je2be::LevelDirectoryStructure::Paper;
  } else if (structureString == "vanilla") {
    structure = je2be::LevelDirectoryStructure::Vanilla;
  } else {
    cerr << "error: unknown LevelDirectoryStructure name; should be \"vanilla\" or \"paper\"" << endl;
    return -1;
  }

  unsigned int concurrency = result["n"].as<unsigned int>();

  auto start = chrono::high_resolution_clock::now();
  defer {
    auto elapsed = chrono::high_resolution_clock::now() - start;
    cout << float(chrono::duration_cast<chrono::milliseconds>(elapsed).count() / 1000.0f) << "s" << endl;
  };

  Options options;
  options.fLevelDirectoryStructure = structure;
  options.fTempDirectory = mcfile::File::CreateTempDir(fs::temp_directory_path());
  options.fDbTempDirectory = mcfile::File::CreateTempDir(fs::temp_directory_path());
  defer {
    if (options.fTempDirectory) {
      je2be::Fs::DeleteAll(*options.fTempDirectory);
    }
    if (options.fDbTempDirectory) {
      je2be::Fs::DeleteAll(*options.fDbTempDirectory);
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
  struct StdoutProgressReporter : public Progress {
    StdoutProgressReporter() : fLast(std::chrono::high_resolution_clock::now()) {}

    bool reportConvert(double progress, u64 numConvertedChunks) override {
      auto now = chrono::high_resolution_clock::now();
      lock_guard<mutex> lock(fMut);
      fNumConvertedChunks = std::max(numConvertedChunks, fNumConvertedChunks);
      if (progress >= 1) {
        printConvertProgress(1);
        cout << "          " << endl;
        fLast = {};
      } else if (now - fLast > chrono::seconds(1)) {
        printConvertProgress(progress);
        fLast = now;
      }
      return true;
    }
    void printConvertProgress(double progress) {
      cout << "\rConvert: " << float(progress * 100) << "% " << fNumConvertedChunks << " chunks";
    }
    bool reportEntityPostProcess(double progress) override {
      auto now = chrono::high_resolution_clock::now();
      lock_guard<mutex> lock(fMut);
      if (progress >= 1) {
        printPostProcessProgress(1);
        cout << "          " << endl;
        fLast = {};
      } else if (now - fLast > chrono::seconds(1)) {
        printPostProcessProgress(progress);
        fLast = now;
      }
      return true;
    }
    void printPostProcessProgress(double progress) {
      cout << "\rPostProcess: " << float(progress * 100) << "%";
    }
    bool reportCompaction(double progress) override {
      auto now = chrono::high_resolution_clock::now();
      lock_guard<mutex> lock(fMut);
      if (progress >= 1) {
        printCompactionProgress(1);
        cout << "          " << endl;
        fLast = {};
      } else if (now - fLast > chrono::seconds(1)) {
        printCompactionProgress(progress);
        fLast = now;
      }
      return true;
    }
    void printCompactionProgress(double progress) {
      cout << "\rCompaction: " << float(progress * 100) << "%";
    }

    mutex fMut;
    chrono::high_resolution_clock::time_point fLast;
    u64 fNumConvertedChunks = 0;
  } progress;
  auto st = Converter::Run(input, output, options, concurrency, &progress);
  return st.ok() ? 0 : -1;
}
