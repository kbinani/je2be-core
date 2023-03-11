#pragma once

#include <je2be/integers.hpp>
#include <je2be/pos2.hpp>

#include <deque>
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
    deque<Span> &spans = fSpans[z];
    auto found = Find(spans, x);
    if (!found) {
      Span s;
      s.fFrom = x;
      s.fTo = x;
      spans.insert(spans.end(), s);
      fSize++;
      return;
    }
    Span span = spans[*found];
    if (span.fFrom <= x && x <= span.fTo) {
      return;
    }
    assert(x < span.fFrom);
    if (x + 1 == span.fFrom) {
      spans[*found].fFrom = x;
      fSize++;
      return;
    }
    Span s;
    s.fFrom = x;
    s.fTo = x;
    spans.insert(spans.begin() + *found, s);
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
    std::unordered_map<i32, std::deque<Span>>::const_iterator fItrZ;
    std::deque<Span>::const_iterator fItrSpan;
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
    auto it = Find(found->second, x);
    if (!it) {
      return end();
    }
    Span span = found->second[*it];
    if (span.fFrom <= x && x <= span.fTo) {
      ConstIterator itr;
      itr.fThis = this;
      itr.fItrZ = found;
      itr.fItrSpan = found->second.begin() + (*it);
      itr.fItrX = x;
      return itr;
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
  static std::optional<size_t> Find(std::deque<Span> const &spans, i32 x) {
    if (spans.empty()) {
      return std::nullopt;
    }
    size_t left = 0;
    size_t right = spans.size() - 1;
    Span spanL = spans[left];
    Span spanR = spans[right];
    if (x <= spanL.fTo) {
      return 0;
    }
    if (spanR.fFrom <= x && x <= spanR.fTo) {
      return right;
    }
    if (spanR.fTo < x) {
      return std::nullopt;
    }
    assert(spanL.fTo < x && x < spanR.fFrom);
    while (true) {
      if (left + 1 == right) {
        return right;
      }
      size_t mid = (left + right) / 2;
      Span spanM = spans[mid];
      if (spanM.fFrom <= x && x <= spanM.fTo) {
        return mid;
      }
      if (x < spanM.fFrom) {
        right = mid;
        spanR = spanM;
      } else {
        left = mid;
        spanL = spanM;
      }
    }
  }

  static void MergeAdjacentSpans(std::deque<Span> &spans) {
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
  std::unordered_map<i32, std::deque<Span>> fSpans;
  size_t fSize = 0;
};

} // namespace je2be
