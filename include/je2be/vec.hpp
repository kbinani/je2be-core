#pragma once

namespace j2b {

class Vec {
public:
    Vec(float x, float y, float z) : fX(x), fY(y), fZ(z) {}

    std::shared_ptr<mcfile::nbt::ListTag> toListTag() const {
        using namespace mcfile::nbt;
        auto tag = std::make_shared<ListTag>();
        tag->fType = Tag::TAG_Float;
        tag->fValue = {std::make_shared<FloatTag>(fX), std::make_shared<FloatTag>(fY), std::make_shared<FloatTag>(fZ)};
        return tag;
    }

public:
    float fX;
    float fY;
    float fZ;
};

}
