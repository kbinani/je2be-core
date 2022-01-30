#pragma once

namespace je2be {

class Lab {
public:
  double fL;
  double fA;
  double fB;

public:
  // https://github.com/antimatter15/rgb-lab/blob/master/color.js
  static Lab From(Rgba rgb) {
    double r = rgb.fR / 255.0;
    double g = rgb.fG / 255.0;
    double b = rgb.fB / 255.0;

    r = (r > 0.04045) ? pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
    g = (g > 0.04045) ? pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
    b = (b > 0.04045) ? pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

    double x = (r * 0.4124 + g * 0.3576 + b * 0.1805) / 0.95047;
    double y = (r * 0.2126 + g * 0.7152 + b * 0.0722) / 1.00000;
    double z = (r * 0.0193 + g * 0.1192 + b * 0.9505) / 1.08883;

    x = (x > 0.008856) ? pow(x, 1.0 / 3.0) : (7.787 * x) + 16.0 / 116.0;
    y = (y > 0.008856) ? pow(y, 1.0 / 3.0) : (7.787 * y) + 16.0 / 116.0;
    z = (z > 0.008856) ? pow(z, 1.0 / 3.0) : (7.787 * z) + 16.0 / 116.0;

    Lab ret;
    ret.fL = (116 * y) - 16;
    ret.fA = 500 * (x - y);
    ret.fB = 200 * (y - z);
    return ret;
  }

  static double Difference(Lab const &labA, Lab const &labB) {
    double deltaL = labA.fL - labB.fL;
    double deltaA = labA.fA - labB.fA;
    double deltaB = labA.fB - labB.fB;
    double c1 = sqrt(labA.fA * labA.fA + labA.fB * labA.fB);
    double c2 = sqrt(labB.fA * labB.fA + labB.fB * labB.fB);
    double deltaC = c1 - c2;
    double deltaH = deltaA * deltaA + deltaB * deltaB - deltaC * deltaC;
    deltaH = deltaH < 0 ? 0 : sqrt(deltaH);
    double sc = 1.0 + 0.045 * c1;
    double sh = 1.0 + 0.015 * c1;
    double deltaLKlsl = deltaL / (1.0);
    double deltaCkcsc = deltaC / (sc);
    double deltaHkhsh = deltaH / (sh);
    double i = deltaLKlsl * deltaLKlsl + deltaCkcsc * deltaCkcsc + deltaHkhsh * deltaHkhsh;
    return i < 0 ? 0 : sqrt(i);
  }

  static std::function<bool(Rgba const &lhs, Rgba const &rhs)> CompareBySimirality(Rgba const &base) {
    Lab ref = Lab::From(base);
    return [ref](Rgba const &lhs, Rgba const &rhs) {
      double dl = Lab::Difference(Lab::From(lhs), ref);
      double dr = Lab::Difference(Lab::From(rhs), ref);
      return dl < dr;
    };
  }
};

} // namespace je2be
