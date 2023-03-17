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
    optional<Rational<u64>> fConvert;
    u64 fNumConvertedChunks = 0;
    optional<Rational<u64>> fPostProcess;
    optional<Rational<u64>> fCompaction;
  };

  StdoutProgressReporter() {
    fIo.reset(new thread([this]() {
      State prev;
      unique_ptr<pbar::pbar> convert;
      unique_ptr<pbar::pbar> postProcess;
      unique_ptr<pbar::pbar> compaction;

      while (!fStop) {
        State s;
        {
          lock_guard<mutex> lock(fMut);
          s = fState;
        }
        if (s.fConvert) {
          if (!convert) {
            convert.reset(new pbar::pbar(s.fConvert->fDen, "Convert"));
            convert->enable_recalc_console_width(10);
          }
          if (prev.fConvert) {
            if (prev.fConvert->fNum < prev.fConvert->fDen) {
              convert->tick(s.fConvert->fNum - prev.fConvert->fNum);
            }
          } else {
            convert->tick(s.fConvert->fNum);
          }
        }
        if (s.fPostProcess) {
          if (!postProcess) {
            postProcess.reset(new pbar::pbar(s.fPostProcess->fDen, "PostProcess"));
            postProcess->enable_recalc_console_width(10);
          }
          if (prev.fPostProcess) {
            if (prev.fPostProcess->fNum < prev.fPostProcess->fDen) {
              postProcess->tick(s.fPostProcess->fNum - prev.fPostProcess->fNum);
            }
          } else {
            postProcess->tick(s.fPostProcess->fNum);
          }
        }
        if (s.fCompaction) {
          if (!compaction) {
            compaction.reset(new pbar::pbar(s.fCompaction->fDen, "LevelDB Compaction"));
            compaction->enable_recalc_console_width(10);
          }
          if (prev.fCompaction) {
            if (prev.fCompaction->fNum < prev.fCompaction->fDen) {
              compaction->tick(s.fCompaction->fNum - prev.fCompaction->fNum);
            }
          } else {
            compaction->tick(s.fCompaction->fNum);
          }
        }
        prev = s;
        this_thread::sleep_for(chrono::milliseconds(16));
      }
    }));
  }

  ~StdoutProgressReporter() {
    fStop = true;
    fIo->join();
    fIo.reset();
  }

  bool reportConvert(Rational<u64> const &progress, u64 numConvertedChunks) override {
    lock_guard<mutex> lock(fMut);
    if (fState.fConvert) {
      fState.fConvert->fNum = std::max(fState.fConvert->fNum, progress.fNum);
    } else {
      fState.fConvert = progress;
    }
    fState.fNumConvertedChunks = numConvertedChunks;
    return true;
  }

  bool reportEntityPostProcess(Rational<u64> const &progress) override {
    lock_guard<mutex> lock(fMut);
    if (fState.fPostProcess) {
      fState.fPostProcess->fNum = std::max(fState.fPostProcess->fNum, progress.fNum);
    } else {
      fState.fPostProcess = progress;
    }
    return true;
  }

  bool reportCompaction(Rational<u64> const &progress) override {
    lock_guard<mutex> lock(fMut);
    if (fState.fCompaction) {
      fState.fCompaction->fNum = std::max(fState.fCompaction->fNum, progress.fNum);
    } else {
      fState.fCompaction = progress;
    }
    return true;
  }

  mutex fMut;
  unique_ptr<thread> fIo;
  atomic_bool fStop = false;
  State fState;
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
  if (auto err = st.error(); err) {
    cerr << "what: " << err->fWhat << endl;
    cerr << "trace: " << endl;
    for (int i = err->fTrace.size() - 1; i >= 0; i--) {
      cerr << "  " << err->fTrace[i].fFile << ":" << err->fTrace[i].fLine << endl;
    }
    return -1;
  } else {
    cout << progress->fState.fNumConvertedChunks << " chunks converted" << endl;
    return 0;
  }
}
