#pragma once

namespace je2be {

class Portal {
public:
  Portal(int32_t dimId, uint8_t span, int32_t tpX, int32_t tpY, int32_t tpZ, uint8_t xa, uint8_t za) : fDimId(dimId), fSpan(span), fTpX(tpX), fTpY(tpY), fTpZ(tpZ), fXa(xa), fZa(za) {}

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

  static std::optional<Portal> FromCompound(CompoundTag const &tag) {
    auto dimId = tag.int32("DimId");
    auto span = tag.byte("Span");
    auto tpX = tag.int32("TpX");
    auto tpY = tag.int32("TpY");
    auto tpZ = tag.int32("TpZ");
    auto xa = tag.byte("Xa");
    auto za = tag.byte("Za");
    if (!dimId || !span || !tpX || !tpY || !tpZ || !xa || !za) {
      return std::nullopt;
    }
    return Portal(*dimId, *span, *tpX, *tpY, *tpZ, *xa, *za);
  }

  int32_t const fDimId;
  uint8_t const fSpan;
  int32_t const fTpX;
  int32_t const fTpY;
  int32_t const fTpZ;
  uint8_t const fXa;
  uint8_t const fZa;
};

} // namespace je2be
