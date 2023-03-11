#pragma once

#include <je2be/integers.hpp>
#include <je2be/pos2.hpp>

#include <unordered_set>

namespace je2be {

class Pos2iSet {
  struct Span {
    i32 fFrom;
    i32 fTo;
  };

public:
  void insert(Pos2i const &p) {
    using namespace std;
    i32 x = p.fX;
    i32 z = p.fZ;
    list<Span> &spans = fSpans[z];
    for (auto it = spans.begin(); it != spans.end(); it++) {
      if (it->fFrom <= x && x <= it->fTo) {
        return;
      }
      if (x + 1 == it->fFrom) {
        it->fFrom = x;
        MergeAdjacentSpans(spans);
        fSize++;
        return;
      }
      if (x == it->fTo + 1) {
        it->fTo = x;
        MergeAdjacentSpans(spans);
        fSize++;
        return;
      }
      if (x < it->fFrom) {
        Span span;
        span.fFrom = x;
        span.fTo = x;
        spans.insert(it, span);
        fSize++;
        return;
      }
    }
    Span s;
    s.fFrom = x;
    s.fTo = x;
    spans.insert(spans.end(), s);
    fSize++;
  }

  size_t size() const {
    return fSize;
  }

  bool empty() const {
    return fSize == 0;
  }

  class ConstIterator {
    friend class Pos2iSet;

    Pos2iSet const *fThis;
    std::unordered_map<i32, std::list<Span>>::const_iterator fItrZ;
    std::list<Span>::const_iterator fItrSpan;
    i32 fItrX;

  public:
    bool operator==(ConstIterator const &o) const {
      if (this == &o) {
        return true;
      }
      if (this->fThis == nullptr && o.fThis == nullptr) {
        return true;
      }
      if (this->fThis != o.fThis) {
        return false;
      }
      return this->fItrZ == o.fItrZ && this->fItrSpan == o.fItrSpan && this->fItrX == o.fItrX;
    }

    ConstIterator &operator++() {
      if (!fThis) {
        return *this;
      }
      fItrX++;
      if (fItrX > fItrSpan->fTo) {
        fItrSpan++;
        if (fItrSpan == fItrZ->second.end()) {
          fItrZ++;
          if (fItrZ == fThis->fSpans.end()) {
            fThis = nullptr;
          } else {
            fItrSpan = fItrZ->second.begin();
            fItrX = fItrSpan->fFrom;
          }
        } else {
          fItrX = fItrSpan->fFrom;
        }
      }
      return *this;
    }

    ConstIterator operator++(int) {
      ConstIterator temp = *this;
      ++*this;
      return temp;
    }

    Pos2i operator*() const {
      i32 x = fItrX;
      i32 z = fItrZ->first;
      return Pos2i(x, z);
    }
  };

  ConstIterator begin() const {
    if (fSpans.empty()) {
      return end();
    } else {
      ConstIterator itr;
      itr.fThis = this;
      itr.fItrZ = fSpans.begin();
      itr.fItrSpan = itr.fItrZ->second.begin();
      itr.fItrX = itr.fItrSpan->fFrom;
      return itr;
    }
  }

  ConstIterator end() const {
    ConstIterator itr;
    itr.fThis = nullptr;
    return itr;
  }

  ConstIterator find(Pos2i const &p) const {
    i32 x = p.fX;
    i32 z = p.fZ;
    auto found = fSpans.find(z);
    if (found == fSpans.end()) {
      return end();
    }
    for (auto it = found->second.begin(); it != found->second.end(); it++) {
      Span span = *it;
      if (span.fFrom <= x && x <= span.fTo) {
        ConstIterator itr;
        itr.fThis = this;
        itr.fItrZ = found;
        itr.fItrSpan = it;
        itr.fItrX = x;
        return itr;
      }
      if (x < span.fFrom) {
        return end();
      }
    }
    return end();
  }

  size_t count(Pos2i const &p) const {
    if (find(p) == end()) {
      return 0;
    } else {
      return 1;
    }
  }

private:
  static void MergeAdjacentSpans(std::list<Span> &spans) {
    auto it = spans.begin();
    while (true) {
      Span s0 = *it;
      auto next = std::next(it);
      if (next == spans.end()) {
        break;
      }
      Span s1 = *next;
      if (s0.fTo + 1 == s1.fFrom) {
        spans.erase(next);
        s0.fTo = s1.fTo;
        *it = s0;
      }
    }
  }

private:
  std::unordered_map<i32, std::list<Span>> fSpans;
  size_t fSize = 0;
};

} // namespace je2be
