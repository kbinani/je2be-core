#pragma once

namespace j2e {

class Version {
public:
    int fMajor;
    int fMinor;
    int fPatch;
    int fRevision;
    int fReserved;

    Version(int major, int minor, int patch, int revision, int reserved)
        : fMajor(major)
        , fMinor(minor)
        , fPatch(patch)
        , fRevision(revision)
        , fReserved(reserved)
    {
    }

    std::shared_ptr<mcfile::nbt::ListTag> toListTag() const {
        using namespace props;

        auto l = std::make_shared<mcfile::nbt::ListTag>();
        l->fType = mcfile::nbt::Tag::TAG_Int;
        l->fValue = {
            Int(fMajor),
            Int(fMinor),
            Int(fPatch),
            Int(fRevision),
            Int(fReserved),
        };
        return l;
    }
};

}
