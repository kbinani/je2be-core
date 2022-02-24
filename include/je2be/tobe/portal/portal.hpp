#pragma once

namespace je2be::tobe {

class Portal {
public:
  Portal(int32_t dimId, uint8_t span, int32_t tpX, int32_t tpY, int32_t tpZ, uint8_t xa, uint8_t za) : fDimId(dimId), fSpan(span), fTpX(tpX), fTpY(tpY), fTpZ(tpZ), fXa(xa), fZa(za) {}

  std::shared_ptr<CompoundTag> toCompoundTag() const {
    using namespace std;
    using namespace je2be::nbt;
    auto tag = make_shared<CompoundTag>();
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

private:
  int32_t const fDimId;
  uint8_t const fSpan;
  int32_t const fTpX;
  int32_t const fTpY;
  int32_t const fTpZ;
  uint8_t const fXa;
  uint8_t const fZa;
};

} // namespace je2be::tobe
