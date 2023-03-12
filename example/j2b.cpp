#include <cxxopts.hpp>
#include <je2be.hpp>

#if __has_include(<mimalloc.h>)
#include <mimalloc.h>
#endif
#include <pbar.hpp>

#include <iostream>
#include <string>
#include <thread>

using namespace std;
using namespace je2be;
using namespace je2be::tobe;
namespace fs = std::filesystem;

struct StdoutProgressReporter : public Progress {
  struct State {
    int fConvert = 0;
    u64 fNumConvertedChunks = 0;
    int fPostProcess = 0;
    int fCompaction = 0;
  };

  StdoutProgressReporter() {
    fIo.reset(new thread([this]() {
      State prev;
      pbar::pbar convert(kProgressScale);
      pbar::pbar postProcess(kProgressScale);
      pbar::pbar compaction(kProgressScale);
      convert.set_description("Convert");
      postProcess.set_description("PostProcess");
      compaction.set_description("LevelDB Compaction");

      convert.enable_recalc_console_width(10);
      postProcess.enable_recalc_console_width(10);
      compaction.enable_recalc_console_width(10);

      while (!fStop) {
        State s;
        {
          lock_guard<mutex> lock(fMut);
          s = fState;
        }
        if (prev.fConvert < s.fConvert) {
          convert.tick(s.fConvert - prev.fConvert);
        }
        if (prev.fNumConvertedChunks < s.fNumConvertedChunks) {
          convert.set_description("Convert(" + to_string(s.fNumConvertedChunks) + "chunks)");
        }
        if (prev.fPostProcess < s.fPostProcess) {
          postProcess.tick(s.fPostProcess - prev.fPostProcess);
        }
        if (prev.fCompaction < s.fCompaction) {
          compaction.tick(s.fCompaction - prev.fCompaction);
        }
        prev = s;
        this_thread::sleep_for(chrono::milliseconds(100));
      }
    }));
  }

  ~StdoutProgressReporter() {
    fStop = true;
    fIo->join();
    fIo.reset();
  }

  bool reportConvert(double progress, u64 numConvertedChunks) override {
    lock_guard<mutex> lock(fMut);
    fState.fConvert = std::clamp<int>((int)floor(progress * kProgressScale), 0, kProgressScale);
    fState.fNumConvertedChunks = numConvertedChunks;
    return true;
  }
  bool reportEntityPostProcess(double progress) override {
    lock_guard<mutex> lock(fMut);
    fState.fPostProcess = std::clamp<int>((int)floor(progress * kProgressScale), 0, kProgressScale);
    return true;
  }
  bool reportCompaction(double progress) override {
    lock_guard<mutex> lock(fMut);
    fState.fCompaction = std::clamp<int>((int)floor(progress * kProgressScale), 0, kProgressScale);
    return true;
  }

  mutex fMut;
  u64 fNumConvertedChunks = 0;
  unique_ptr<thread> fIo;
  atomic_bool fStop = false;
  State fState;
  int const kProgressScale = 100000;
};

int main(int argc, char *argv[]) {
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
  unique_ptr<StdoutProgressReporter> progress(new StdoutProgressReporter);
  auto st = Converter::Run(input, output, options, concurrency, progress.get());
  progress.reset();
  return st.ok() ? 0 : -1;
}
