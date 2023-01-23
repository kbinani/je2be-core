#pragma once

namespace je2be {

class Panda : StaticReversibleMap<std::string, i32, Panda> {
  Panda() = delete;

public:
  enum Gene : i32 {
    Lazy,
    Brown,
    Weak,
    Aggressive,
    Normal,
    Playful,
    Worried,
  };

  static ReversibleMap<std::string, i32> const *CreateTable() {
    return new ReversibleMap<std::string, i32>({
        {"lazy", Lazy},
        {"brown", Brown},
        {"weak", Weak},
        {"aggressive", Aggressive},
        {"normal", Normal},
        {"playful", Playful},
        {"worried", Worried},
    });
  }

  static std::string JavaGeneNameFromGene(Gene g) {
    return Backward(g, "normal");
  }

  static Gene GeneFromJavaName(std::string const &name) {
    return (Gene)Forward(name, (i32)Normal);
  }

  static Gene GeneFromBedrockAllele(i32 allele) {
    if (allele == 0) {
      return Lazy;
    } else if (allele == 1) {
      return Worried;
    } else if (allele == 2) {
      return Playful;
    } else if (allele == 3) {
      return Aggressive;
    } else if (4 <= allele && allele <= 7) {
      return Weak;
    } else if (8 <= allele && allele <= 9) {
      return Brown;
    } else {
      return Normal;
    }
  }

  static i32 BedrockAlleleFromGene(Gene g) {
    switch (g) {
    case Lazy:
      return 0;
    case Worried:
      return 1;
    case Playful:
      return 2;
    case Aggressive:
      return 3;
    case Weak:
      return 4;
    case Brown:
      return 8;
    case Normal:
    default:
      return 10;
    }
  }
};

} // namespace je2be
