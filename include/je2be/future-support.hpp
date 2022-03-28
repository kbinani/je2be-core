#pragma once

namespace je2be {
class FutureSupport {
private:
  FutureSupport() = delete;

public:
  template <class T>
  static void Drain(size_t maxRunningTasks, std::deque<std::future<T>> &futures, std::vector<std::future<T>> &drain) {
    using namespace std;
    for (size_t i = 0; i < futures.size(); i++) {
      auto status = futures[i].wait_for(chrono::nanoseconds(0));
      if (status != future_status::ready) {
        continue;
      }
      drain.push_back(future<T>());
      swap(drain[drain.size() - 1], futures[i]);
      futures.erase(futures.begin() + i);
      i--;
    }
    if (futures.size() > maxRunningTasks) {
      size_t pop = futures.size() - maxRunningTasks;
      for (size_t i = 0; i < pop; i++) {
        drain.push_back(future<T>());
        swap(drain[drain.size() - 1], futures.front());
        futures.pop_front();
      }
    }
  }
};

} // namespace je2be
