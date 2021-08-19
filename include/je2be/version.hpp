#pragma once

namespace j2b {

class Version {
public:
  int fMajor;
  int fMinor;
  int fPatch;
  int fRevision;
  int fReserved;

  Version(int major, int minor, int patch, int revision, int reserved) : fMajor(major), fMinor(minor), fPatch(patch), fRevision(revision), fReserved(reserved) {}

  std::shared_ptr<mcfile::nbt::ListTag> toListTag() const {
    using namespace props;

    auto l = std::make_shared<mcfile::nbt::ListTag>(mcfile::nbt::Tag::TAG_Int);
    l->push_back(Int(fMajor));
    l->push_back(Int(fMinor));
    l->push_back(Int(fPatch));
    l->push_back(Int(fRevision));
    l->push_back(Int(fReserved));
    return l;
  }
};

} // namespace j2b
