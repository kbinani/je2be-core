#pragma once

namespace je2be::java {

class Portal {
public:
  Portal(i32 dimId, u8 span, i32 tpX, i32 tpY, i32 tpZ, u8 xa, u8 za) : fDimId(dimId), fSpan(span), fTpX(tpX), fTpY(tpY), fTpZ(tpZ), fXa(xa), fZa(za) {}

  CompoundTagPtr toCompoundTag() const {
    using namespace std;
    auto tag = Compound();
    tag->insert({
        {u8"DimId", Int(fDimId)},
        {u8"Span", Byte(fSpan)},
        {u8"TpX", Int(fTpX)},
        {u8"TpY", Int(fTpY)},
        {u8"TpZ", Int(fTpZ)},
        {u8"Xa", Byte(fXa)},
        {u8"Za", Byte(fZa)},
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

} // namespace je2be::java
