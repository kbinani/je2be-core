#pragma once

#include <je2be/integers.hpp>
#include <memory>
#include <mutex>

struct StdoutProgressReporter : public je2be::lce::Progress {
  struct State {
    std::optional<je2be::Rational<je2be::u64>> fConvert;
  };

  StdoutProgressReporter() {
    using namespace std;
    fIo.reset(new thread([this]() {
      State prev;
      unique_ptr<pbar::pbar> convert;

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
            if (prev.fConvert->fNum < prev.fConvert->fDen && s.fConvert->fNum > prev.fConvert->fNum) {
              convert->tick(s.fConvert->fNum - prev.fConvert->fNum);
            }
          } else {
            convert->tick(s.fConvert->fNum);
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

  bool report(je2be::Rational<je2be::u64> const &progress) override {
    if (progress.fDen > 0) {
      std::lock_guard<std::mutex> lock(fMut);
      if (fState.fConvert) {
        fState.fConvert->fNum = std::max(fState.fConvert->fNum, progress.fNum);
      } else {
        fState.fConvert = progress;
      }
    }
    return true;
  }

  std::mutex fMut;
  std::unique_ptr<std::thread> fIo;
  std::atomic_bool fStop = false;
  State fState;
};
