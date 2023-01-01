#pragma once

#include <latch>

namespace je2be {

class Parallel {
public:
  template <class Result, class Work>
  static Result ForEach(
      std::vector<Work> const &works,
      Result init,
      std::function<Result(Work const &)> func,
      std::function<void(Result const &, Result &)> join) {
    using namespace std;

    auto threads = thread::hardware_concurrency();
    std::latch latch(threads + 1);

    mutex queueMut;
    vector<bool> done(works.size(), false);

    mutex joinMut;
    Result total = init;

    for (int i = 0; i < threads; i++) {
      thread([init, &latch, &queueMut, &joinMut, &works, func, join, &total, &done]() {
        Result sum = init;
        while (true) {
          Work const *work = nullptr;
          {
            lock_guard<mutex> lock(queueMut);
            for (int j = 0; j < works.size(); j++) {
              if (!done[j]) {
                work = &works[j];
                done[j] = true;
                break;
              }
            }
          }
          if (work) {
            Result result = func(*work);
            join(result, sum);
          } else {
            lock_guard<mutex> lock(joinMut);
            join(sum, total);
            break;
          }
        }
        latch.count_down();
      }).detach();
    }
    latch.arrive_and_wait();
    return total;
  }
};

} // namespace je2be
