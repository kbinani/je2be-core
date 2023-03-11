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

  using Bucket = std::deque<Span>;

public:
  void insert(Pos2i const &p) {
    using namespace std;
    if (insertImpl(p)) {
      fSize++;
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
    Bucket::const_iterator fItrSpan;
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
          if (fItrZ == fThis->fBuckets.end()) {
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
    if (fBuckets.empty()) {
      return end();
    } else {
      ConstIterator itr;
      itr.fThis = this;
      itr.fItrZ = fBuckets.begin();
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
    auto found = fBuckets.find(z);
    if (found == fBuckets.end()) {
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
  static std::optional<size_t> Find(Bucket const &bucket, i32 x) {
    if (bucket.empty()) {
      return std::nullopt;
    }
    size_t left = 0;
    size_t right = bucket.size() - 1;
    Span spanL = bucket[left];
    Span spanR = bucket[right];
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
      Span spanM = bucket[mid];
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

  bool insertImpl(Pos2i const &p) {
    using namespace std;
    i32 x = p.fX;
    i32 z = p.fZ;
    Bucket &bucket = fBuckets[z];
    auto found = Find(bucket, x);
    if (!found) {
      if (!bucket.empty()) {
        Span &back = bucket.back();
        if (back.fTo + 1 == x) {
          back.fTo = x;
          return true;
        }
      }
      Span s;
      s.fFrom = x;
      s.fTo = x;
      bucket.insert(bucket.end(), s);
      return true;
    }
    Span &span = bucket[*found];
    if (span.fFrom <= x && x <= span.fTo) {
      return false;
    }
    assert(x < span.fFrom);
    if (*found > 0) {
      Span &prev = bucket[*found - 1];
      if (prev.fTo + 1 == x) {
        if (x + 1 == span.fFrom) {
          prev.fTo = span.fTo;
          bucket.erase(bucket.begin() + *found);
          return true;
        } else {
          prev.fTo = x;
          return true;
        }
      }
    }
    if (x + 1 == span.fFrom) {
      span.fFrom = x;
      return true;
    }
    Span s;
    s.fFrom = x;
    s.fTo = x;
    bucket.insert(bucket.begin() + *found, s);
    return true;
  }

private:
  std::unordered_map<i32, Bucket> fBuckets;
  size_t fSize = 0;
};

} // namespace je2be
