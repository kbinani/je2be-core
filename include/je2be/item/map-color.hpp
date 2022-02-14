#pragma once

namespace je2be {

class MapColor {
  MapColor() = delete;

  static std::vector<Rgba> const *GetTable() {
    static std::unique_ptr<std::vector<Rgba> const> const sTable(CreateTable());
    return sTable.get();
  }

  static std::vector<Rgba> const *CreateTable() {
    return new std::vector<Rgba>({
        Rgba(0, 0, 0, 0),
        Rgba(127, 178, 56),
        Rgba(247, 233, 163),
        Rgba(199, 199, 199),
        Rgba(255, 0, 0),
        Rgba(160, 160, 255),
        Rgba(167, 167, 167),
        Rgba(0, 124, 0),
        Rgba(255, 255, 255),
        Rgba(164, 168, 184),
        Rgba(151, 109, 77),
        Rgba(112, 112, 112),
        Rgba(64, 64, 255),
        Rgba(143, 119, 72),
        Rgba(255, 252, 245),
        Rgba(216, 127, 51),
        Rgba(178, 76, 216),
        Rgba(102, 153, 216),
        Rgba(229, 229, 51),
        Rgba(127, 204, 25),
        Rgba(242, 127, 165),
        Rgba(76, 76, 76),
        Rgba(153, 153, 153),
        Rgba(76, 127, 153),
        Rgba(127, 63, 178),
        Rgba(51, 76, 178),
        Rgba(102, 76, 51),
        Rgba(102, 127, 51),
        Rgba(153, 51, 51),
        Rgba(25, 25, 25),
        Rgba(250, 238, 77),
        Rgba(92, 219, 213),
        Rgba(74, 128, 255),
        Rgba(0, 217, 58),
        Rgba(129, 86, 49),
        Rgba(112, 2, 0),
        Rgba(209, 177, 161),
        Rgba(159, 82, 36),
        Rgba(149, 87, 108),
        Rgba(112, 108, 138),
        Rgba(186, 133, 36),
        Rgba(103, 117, 53),
        Rgba(160, 77, 78),
        Rgba(57, 41, 35),
        Rgba(135, 107, 98),
        Rgba(87, 92, 92),
        Rgba(122, 73, 88),
        Rgba(76, 62, 92),
        Rgba(76, 50, 35),
        Rgba(76, 82, 42),
        Rgba(142, 60, 46),
        Rgba(37, 22, 16),
        Rgba(189, 48, 49),
        Rgba(148, 63, 97),
        Rgba(92, 25, 29),
        Rgba(22, 126, 134),
        Rgba(58, 142, 140),
        Rgba(86, 44, 62),
        Rgba(20, 180, 133),
        Rgba(100, 100, 100), // 59 DEEPSLATE
        Rgba(216, 175, 147), // 60 RAW_IRON
        Rgba(127, 167, 150), // 61 GLOW_LICHEN
    });
  }

  struct Colors {
    uint8_t fColorId;
    Lab fLab;
  };

  static std::unordered_map<int32_t, Colors> const *CreateLabTable() {
    auto const &table = *GetTable();

    auto ret = new std::unordered_map<int32_t, Colors>();
    uint8_t tableSize = mcfile::Clamp<uint8_t>(table.size());
    for (uint8_t i = 0; i < tableSize; i++) {
      for (uint8_t v = 0; v < 4; v++) {
        Rgba rgb = RgbaFromIndexAndVariant(i, v);
        Lab lab = Lab::From(rgb);
        uint8_t colorId = i * 4 + v;
        Colors cs;
        cs.fColorId = colorId;
        cs.fLab = lab;
        ret->insert(std::make_pair(rgb.toARGB(), cs));
      }
    }
    return ret;
  }

  static std::unordered_map<int32_t, Colors> const *GetLabTable() {
    static std::unique_ptr<std::unordered_map<int32_t, Colors> const> const sTable(CreateLabTable());
    return sTable.get();
  }

  static Rgba RgbaFromIndexAndVariant(uint8_t index, uint8_t variant) {
    auto const &mapping = *GetTable();
    if (index >= mapping.size()) {
      return mapping[0];
    }
    Rgba base = mapping[index];
    static int32_t const mul[4] = {180, 220, 255, 135};
    uint8_t r = (uint8_t)((int32_t)base.fR * mul[variant] / 255);
    uint8_t g = (uint8_t)((int32_t)base.fG * mul[variant] / 255);
    uint8_t b = (uint8_t)((int32_t)base.fB * mul[variant] / 255);
    return Rgba(r, g, b, base.fA);
  }

public:
  static Rgba RgbaFromId(uint8_t colorId) {
    uint8_t variant = 0x3 & colorId;
    uint8_t index = colorId / 4;
    return RgbaFromIndexAndVariant(index, variant);
  }

  static uint8_t MostSimilarColorId(Rgba color) {
    auto const &table = *GetLabTable();

    if (color.fA == 0) {
      return 0;
    }

    int32_t rgba = color.toARGB();
    auto found = table.find(rgba);
    if (found != table.end()) {
      return found->second.fColorId;
    }

    Lab ref = Lab::From(color);
    uint8_t colorId = 0;

    double minDifference = std::numeric_limits<double>::max();
    for (auto const &it : table) {
      Lab lab = it.second.fLab;
      double difference = Lab::Difference(ref, lab);
      if (difference < minDifference) {
        minDifference = difference;
        colorId = it.second.fColorId;
      }
    }
    return colorId;
  }
};
} // namespace je2be
