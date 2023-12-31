#pragma once

#include <je2be/status.hpp>

#include <functional>
#include <latch>
#include <mutex>
#include <thread>
#include <vector>

namespace je2be {

class Parallel {
public:
  template <class Work, class Result>
  static Status Map(
      std::vector<Work> const &works,
      unsigned int concurrency,
      std::function<std::pair<Result, Status>(Work const &work, int index)> func,
      std::vector<Result> &out) {
    using namespace std;

    int numThreads = (int)concurrency - 1;
    std::latch latch(concurrency);

    out.resize(works.size());
    Result *outPtr = out.data();
    Work const *worksPtr = works.data();

    mutex mut;
    vector<bool> done(works.size(), false);
    atomic_bool cancel = false;
    Status status;

    auto action = [&latch, worksPtr, outPtr, &mut, &done, &cancel, &status, func]() {
      while (!cancel) {
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
          auto [ret, st] = func(worksPtr[index], index);
          outPtr[index] = ret;
          if (!st.ok()) {
            lock_guard<mutex> lock(mut);
            if (status.ok()) {
              status = st;
            }
            cancel = true;
            break;
          }
        }
      }
      latch.count_down();
    };

    vector<thread> threads;
    for (int i = 0; i < numThreads; i++) {
      threads.push_back(thread(action));
    }
    action();
    latch.wait();

    for (auto &th : threads) {
      th.join();
    }
    return status;
  }

  template <class Work>
  static Status Process(std::vector<Work> const &works, unsigned int concurrency, std::function<Status(Work const &work)> func) {
    using namespace std;
    auto [ret, status] = Reduce<Work, int>(
        works, concurrency, 0, [func](Work const &work) -> pair<int, Status> { return make_pair(0, func(work)); },
        [](int const &, int) {});
    return status;
  }

  template <class Work, class Result>
  static std::pair<Result, Status> Reduce(
      std::vector<Work> const &works,
      unsigned int concurrency,
      Result init,
      std::function<std::pair<Result, Status>(Work const &)> func,
      std::function<void(Result const &, Result &)> join) {
    return Reduce<Work, Result>(
        works, concurrency, [&init]() { return init; }, func, join);
  }

  template <class Work, class Result>
  static std::pair<Result, Status> Reduce(
      std::vector<Work> const &works,
      unsigned int concurrency,
      std::function<Result(void)> zero,
      std::function<std::pair<Result, Status>(Work const &)> func,
      std::function<void(Result const &, Result &)> join) {
    using namespace std;

    int numThreads = (int)concurrency - 1;
    unique_ptr<std::latch> latch;
    if (concurrency > 0) {
      latch.reset(new std::latch(concurrency));
    }
    std::latch *latchPtr = latch.get();

    vector<shared_ptr<atomic_bool>> done;
    for (int i = 0; i < works.size(); i++) {
      done.push_back(make_shared<atomic_bool>(false));
    }
    shared_ptr<atomic_bool> *donePtr = done.data();

    mutex joinMut;
    Result total = zero();
    atomic_bool cancel = false;
    Status status;

    auto action = [latchPtr, zero, donePtr, &joinMut, &works, &func, join, &total, &cancel, &status]() {
      Result sum = zero();
      while (!cancel) {
        Work const *work = nullptr;
        for (int j = 0; j < works.size(); j++) {
          bool expected = false;
          if (donePtr[j]->compare_exchange_strong(expected, true)) {
            work = &works[j];
            break;
          }
        }
        if (work) {
          auto [result, st] = func(*work);
          join(result, sum);
          if (!st.ok()) {
            lock_guard<mutex> lock(joinMut);
            if (status.ok()) {
              status = st;
            }
            cancel = true;
            break;
          }
        } else {
          lock_guard<mutex> lock(joinMut);
          join(sum, total);
          break;
        }
      }
      if (latchPtr) {
        latchPtr->count_down();
      }
    };

    vector<thread> threads;
    for (int i = 0; i < numThreads; i++) {
      threads.push_back(thread(action));
    }
    action();
    if (latch) {
      latch->wait();
    }

    for (auto &th : threads) {
      th.join();
    }
    return make_pair(total, status);
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
