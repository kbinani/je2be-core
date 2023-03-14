#include <cxxopts.hpp>
#include <je2be.hpp>

#if __has_include(<mimalloc.h>)
#include <mimalloc.h>
#endif
#include <pbar.hpp>

#include <iostream>
#include <thread>

using namespace std;
using namespace je2be;
using namespace je2be::toje;
namespace fs = std::filesystem;

struct StdoutProgressReporter : public Progress {
  struct State {
    optional<Rational<u64>> fConvert;
    u64 fNumConvertedChunks = 0;
    optional<Rational<u64>> fTerraform;
    u64 fNumTerraformedChunks = 0;
  };

  StdoutProgressReporter() {
    fIo.reset(new thread([this]() {
      State prev;
      unique_ptr<pbar::pbar> convert;
      unique_ptr<pbar::pbar> terraform;

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
        if (s.fTerraform) {
          if (!terraform) {
            terraform.reset(new pbar::pbar(s.fTerraform->fDen, "Terraform"));
            terraform->enable_recalc_console_width(10);
          }
          if (prev.fTerraform) {
            if (prev.fTerraform->fNum < prev.fTerraform->fDen) {
              terraform->tick(s.fTerraform->fNum - prev.fTerraform->fNum);
            }
          } else {
            terraform->tick(s.fTerraform->fNum);
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

  bool reportTerraform(Rational<u64> const &progress, u64 numProcessedChunks) override {
    lock_guard<mutex> lock(fMut);
    if (fState.fTerraform) {
      fState.fTerraform->fNum = std::max(fState.fTerraform->fNum, progress.fNum);
    } else {
      fState.fTerraform = progress;
    }
    fState.fNumTerraformedChunks = numProcessedChunks;
    return true;
  }

  mutex fMut;
  unique_ptr<thread> fIo;
  State fState;
  atomic_bool fStop = false;
};

int main(int argc, char *argv[]) {
#if __has_include(<mimalloc.h>)
  mi_version();
#endif

  cxxopts::Options parser("b2j");
  parser.add_options()                                    //
      ("i", "input directory", cxxopts::value<string>())  //
      ("o", "output directory", cxxopts::value<string>()) //
      ("n", "num threads", cxxopts::value<unsigned int>()->default_value(to_string(thread::hardware_concurrency())));
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
  unique_ptr<StdoutProgressReporter> progress(new StdoutProgressReporter);
  auto st = Converter::Run(input, output, options, concurrency, progress.get());
  if (auto err = st.error(); err) {
    cerr << err->fWhat << endl;
    cerr << err->fWhere.fFile << ":" << err->fWhere.fLine << endl;
    return -1;
  } else {
    cout << progress->fState.fNumConvertedChunks << " chunks converted" << endl;
    cout << progress->fState.fNumTerraformedChunks << " chunks terraformed" << endl;
    return 0;
  }
}
