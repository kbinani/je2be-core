#pragma once

namespace je2be {

class Version {
public:
  int fMajor;
  int fMinor;
  int fPatch;
  int fRevision;
  int fReserved;

  constexpr Version(int major, int minor, int patch, int revision, int reserved) : fMajor(major), fMinor(minor), fPatch(patch), fRevision(revision), fReserved(reserved) {}

  ListTagPtr toListTag() const {
    auto l = List<Tag::Type::Int>();
    l->push_back(Int(fMajor));
    l->push_back(Int(fMinor));
    l->push_back(Int(fPatch));
    l->push_back(Int(fRevision));
    l->push_back(Int(fReserved));
    return l;
  }
};

} // namespace je2be
