#pragma once

namespace je2be::tobe {

class Portal {
public:
  Portal(i32 dimId, u8 span, i32 tpX, i32 tpY, i32 tpZ, u8 xa, u8 za) : fDimId(dimId), fSpan(span), fTpX(tpX), fTpY(tpY), fTpZ(tpZ), fXa(xa), fZa(za) {}

  CompoundTagPtr toCompoundTag() const {
    using namespace std;
    auto tag = Compound();
    tag->insert({
        {"DimId", Int(fDimId)},
        {"Span", Byte(fSpan)},
        {"TpX", Int(fTpX)},
        {"TpY", Int(fTpY)},
        {"TpZ", Int(fTpZ)},
        {"Xa", Byte(fXa)},
        {"Za", Byte(fZa)},
    });
    return tag;
  }

  i32 const fDimId;
  u8 const fSpan;
  i32 const fTpX;
  i32 const fTpY;
  i32 const fTpZ;
  u8 const fXa;
  u8 const fZa;
};

} // namespace je2be::tobe
