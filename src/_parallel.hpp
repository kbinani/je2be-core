#pragma once

#include <latch>
#include <thread>
#include <vector>

namespace je2be {

class Parallel {
public:
  template <class Work, class Result>
  static void Map(
      std::vector<Work> const &works,
      unsigned int concurrency,
      std::function<Result(Work const &work, int index)> func,
      std::vector<Result> &out) {
    using namespace std;

    std::latch latch(concurrency + 1);

    out.resize(works.size());
    Result *outPtr = out.data();
    Work const *worksPtr = works.data();

    mutex mut;
    vector<bool> done(works.size(), false);
    vector<thread> threads;
    threads.resize(concurrency);

    for (int i = 0; i < concurrency; i++) {
      thread([&latch, worksPtr, outPtr, &mut, &done, func]() {
        while (true) {
          int index = -1;
          {
            lock_guard<mutex> lock(mut);
            for (int j = 0; j < done.size(); j++) {
              if (!done[j]) {
                index = j;
                done[j] = true;
                break;
              }
            }
          }
          if (index < 0) {
            break;
          } else {
            outPtr[index] = func(worksPtr[index], index);
          }
        }
        latch.count_down();
      }).swap(threads[i]);
    }
    latch.arrive_and_wait();
    for (auto &th : threads) {
      th.join();
    }
  }

  template <class Work, class Result>
  static Result Reduce(
      std::vector<Work> const &works,
      unsigned int concurrency,
      Result init,
      std::function<Result(Work const &)> func,
      std::function<void(Result const &, Result &)> join) {
    return Reduce<Work, Result>(
        works, concurrency, [&init]() { return init; }, func, join);
  }

  template <class Work, class Result>
  static Result Reduce(
      std::vector<Work> const &works,
      unsigned int concurrency,
      std::function<Result(void)> zero,
      std::function<Result(Work const &)> func,
      std::function<void(Result const &, Result &)> join) {
    using namespace std;

    std::latch latch(concurrency + 1);

    vector<shared_ptr<atomic_bool>> done;
    for (int i = 0; i < works.size(); i++) {
      done.push_back(make_shared<atomic_bool>(false));
    }
    shared_ptr<atomic_bool> *donePtr = done.data();

    vector<thread> threads;
    threads.resize(concurrency);

    mutex joinMut;
    Result total = zero();

    for (int i = 0; i < concurrency; i++) {
      thread([zero, &latch, donePtr, &joinMut, &works, func, join, &total]() {
        Result sum = zero();
        while (true) {
          Work const *work = nullptr;
          for (int j = 0; j < works.size(); j++) {
            bool expected = false;
            if (donePtr[j]->compare_exchange_strong(expected, true)) {
              work = &works[j];
              break;
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
      }).swap(threads[i]);
    }
    latch.arrive_and_wait();
    for (auto &th : threads) {
      th.join();
    }
    return total;
  }

  static void MergeBool(bool const &from, bool &to) {
    to = from && to;
  }

  template <class T>
  static void MergeVector(std::vector<T> const &from, std::vector<T> &to) {
    std::copy(from.begin(), from.end(), std::back_inserter(to));
  }
};

} // namespace je2be
