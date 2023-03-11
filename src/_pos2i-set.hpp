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

  struct Bucket {
    std::deque<Span> fSpans;
    size_t fUnmergedInserts = 0;

    std::optional<size_t> find(i32 x) const {
      if (fSpans.empty()) {
        return std::nullopt;
      }
      size_t left = 0;
      size_t right = fSpans.size() - 1;
      Span spanL = fSpans[left];
      Span spanR = fSpans[right];
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
        Span spanM = fSpans[mid];
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

    void mergeAdjacentSpans() {
      auto it = fSpans.begin();
      while (true) {
        Span s0 = *it;
        auto next = std::next(it);
        if (next == fSpans.end()) {
          break;
        }
        Span s1 = *next;
        if (s0.fTo + 1 == s1.fFrom) {
          fSpans.erase(next);
          s0.fTo = s1.fTo;
          *it = s0;
        }
      }
    }
  };

public:
  void insert(Pos2i const &p) {
    using namespace std;
    if (Bucket *bucket = insertImpl(p); bucket) {
      fSize++;
      if (bucket->fUnmergedInserts >= 100) {
        bucket->mergeAdjacentSpans();
        bucket->fUnmergedInserts = 0;
      }
    }
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
    std::unordered_map<i32, Bucket>::const_iterator fItrZ;
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
        if (fItrSpan == fItrZ->second.fSpans.end()) {
          fItrZ++;
          if (fItrZ == fThis->fBuckets.end()) {
            fThis = nullptr;
          } else {
            fItrSpan = fItrZ->second.fSpans.begin();
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
    if (fBuckets.empty()) {
      return end();
    } else {
      ConstIterator itr;
      itr.fThis = this;
      itr.fItrZ = fBuckets.begin();
      itr.fItrSpan = itr.fItrZ->second.fSpans.begin();
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
    auto found = fBuckets.find(z);
    if (found == fBuckets.end()) {
      return end();
    }
    auto it = found->second.find(x);
    if (!it) {
      return end();
    }
    Span span = found->second.fSpans[*it];
    if (span.fFrom <= x && x <= span.fTo) {
      ConstIterator itr;
      itr.fThis = this;
      itr.fItrZ = found;
      itr.fItrSpan = found->second.fSpans.begin() + (*it);
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
  Bucket *insertImpl(Pos2i const &p) {
    using namespace std;
    i32 x = p.fX;
    i32 z = p.fZ;
    Bucket &bucket = fBuckets[z];
    auto found = bucket.find(x);
    if (!found) {
      Span s;
      s.fFrom = x;
      s.fTo = x;
      bucket.fSpans.insert(bucket.fSpans.end(), s);
      bucket.fUnmergedInserts++;
      return &bucket;
    }
    Span span = bucket.fSpans[*found];
    if (span.fFrom <= x && x <= span.fTo) {
      return nullptr;
    }
    assert(x < span.fFrom);
    if (x + 1 == span.fFrom) {
      bucket.fSpans[*found].fFrom = x;
      return &bucket;
    }
    Span s;
    s.fFrom = x;
    s.fTo = x;
    bucket.fSpans.insert(bucket.fSpans.begin() + *found, s);
    bucket.fUnmergedInserts++;
    return &bucket;
  }

private:
  std::unordered_map<i32, Bucket> fBuckets;
  size_t fSize = 0;
};

} // namespace je2be
