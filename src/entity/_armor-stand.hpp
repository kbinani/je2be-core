#pragma once

#include <je2be/nbt.hpp>

#include "_rotation.hpp"

namespace je2be {

class ArmorStand {
public:
  using Rot = std::tuple<float, float, float>;

  struct Pose {
    Rot fBody;
    Rot fHead;
    Rot fLeftLeg;
    Rot fRightLeg;
    Rot fLeftArm;
    Rot fRightArm;

    static ListTagPtr ListTagFromRot(Rot r) {
      auto ret = List<Tag::Type::Float>();
      ret->push_back(Float(std::get<0>(r)));
      ret->push_back(Float(std::get<1>(r)));
      ret->push_back(Float(std::get<2>(r)));
      return ret;
    }

    static void SetRot(Rot rot, std::u8string const &key, CompoundTag &dest) {
      using namespace std;
      if (get<0>(rot) == 0 && get<1>(rot) == 0 && get<2>(rot) == 0) {
        return;
      }
      dest[key] = ListTagFromRot(rot);
    }

    static Rot GetRot(std::u8string const &key, CompoundTag const &src) {
      Rot zero{0.0f, 0.0f, 0.0f};
      auto l = src.listTag(key);
      if (!l) {
        return zero;
      }
      if (l->fType != Tag::Type::Float || l->size() != 3) {
        return zero;
      }
      return Rot(l->at(0)->asFloat()->fValue, l->at(1)->asFloat()->fValue, l->at(2)->asFloat()->fValue);
    }

    CompoundTagPtr toCompoundTag() const {
      auto ret = Compound();
      SetRot(fBody, u8"Body", *ret);
      SetRot(fHead, u8"Head", *ret);
      SetRot(fLeftLeg, u8"LeftLeg", *ret);
      SetRot(fRightLeg, u8"RightLeg", *ret);
      SetRot(fLeftArm, u8"LeftArm", *ret);
      SetRot(fRightArm, u8"RightArm", *ret);
      return ret;
    }

    static Pose FromCompoundTag(CompoundTag const &pose) {
      Pose p;
      p.fBody = GetRot(u8"Body", pose);
      p.fHead = GetRot(u8"Head", pose);
      p.fLeftLeg = GetRot(u8"LeftLeg", pose);
      p.fRightLeg = GetRot(u8"RightLeg", pose);
      p.fLeftArm = GetRot(u8"LeftArm", pose);
      p.fRightArm = GetRot(u8"RightArm", pose);
      return p;
    }
  };

  static std::vector<Pose> const &GetJavaPoses() {
    static std::vector<Pose> const sPoses = {
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {0.0f, 0.0f, 0.0f}, .fLeftLeg = {0.0f, 0.0f, 0.0f}, .fRightLeg = {0.0f, 0.0f, 0.0f}, .fLeftArm = {351.0f, 0.0f, 340.0f}, .fRightArm = {351.0f, 0.0f, 20.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {0.0f, 0.0f, 0.0f}, .fLeftLeg = {0.0f, 0.0f, 0.0f}, .fRightLeg = {0.0f, 0.0f, 0.0f}, .fLeftArm = {0.0f, 0.0f, 0.0f}, .fRightArm = {0.0f, 0.0f, 0.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {16.0f, 0.0f, 0.0f}, .fLeftLeg = {0.0f, 0.0f, 0.0f}, .fRightLeg = {0.0f, 0.0f, 0.0f}, .fLeftArm = {330.0f, 18.0f, 0.0f}, .fRightArm = {303.0f, 338.0f, 0.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {0.0f, 0.0f, 0.0f}, .fLeftLeg = {0.0f, 0.0f, 0.0f}, .fRightLeg = {0.0f, 0.0f, 0.0f}, .fLeftArm = {16.0f, 18.0f, 0.0f}, .fRightArm = {303.0f, 11.0f, 12.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {357.0f, 0.0f, 0.0f}, .fLeftLeg = {10.0f, 352.0f, 0.0f}, .fRightLeg = {351.0f, 11.0f, 0.0f}, .fLeftArm = {16.0f, 18.0f, 0.0f}, .fRightArm = {241.0f, 31.0f, 12.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {357.0f, 0.0f, 0.0f}, .fLeftLeg = {10.0f, 352.0f, 0.0f}, .fRightLeg = {351.0f, 11.0f, 0.0f}, .fLeftArm = {241.0f, 38.0f, 0.0f}, .fRightArm = {241.0f, 318.0f, 0.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {357.0f, 0.0f, 0.0f}, .fLeftLeg = {10.0f, 352.0f, 0.0f}, .fRightLeg = {351.0f, 11.0f, 0.0f}, .fLeftArm = {241.0f, 318.0f, 0.0f}, .fRightArm = {241.0f, 38.0f, 0.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {0.0f, 0.0f, 0.0f}, .fLeftLeg = {0.0f, 0.0f, 0.0f}, .fRightLeg = {0.0f, 0.0f, 0.0f}, .fLeftArm = {16.0f, 0.0f, 0.0f}, .fRightArm = {296.0f, 318.0f, 0.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {16.0f, 11.0f, 0.0f}, .fLeftLeg = {0.0f, 38.0f, 353.0f}, .fRightLeg = {10.0f, 11.0f, 6.0f}, .fLeftArm = {180.0f, 86.0f, 60.0f}, .fRightArm = {282.0f, 331.0f, 0.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {351.0f, 345.0f, 0.0f}, .fLeftLeg = {16.0f, 0.0f, 0.0f}, .fRightLeg = {330.0f, 0.0f, 0.0f}, .fLeftArm = {255.0f, 4.0f, 360.0f}, .fRightArm = {255.0f, 352.0f, 0.0f}},
        {.fBody = {0.0f, 4.0f, 6.0f}, .fHead = {357.0f, 106.0f, 12.0f}, .fLeftLeg = {255.0f, 45.0f, 0.0f}, .fRightLeg = {3.0f, 31.0f, 0.0f}, .fLeftArm = {173.0f, 52.0f, 53.0f}, .fRightArm = {201.0f, 113.0f, 319.0f}},
        {.fBody = {0.0f, 0.0f, 353.f}, .fHead = {357.0f, 79.0f, 360.0f}, .fLeftLeg = {351.0f, 31.0f, 6.0f}, .fRightLeg = {228.0f, 331.0f, 46.0f}, .fLeftArm = {160.0f, 120.0f, 53.0f}, .fRightArm = {201.0f, 0.0f, 299.0f}},
        {.fBody = {0.0f, 0.0f, 0.0f}, .fHead = {0.0f, 0.0f, 0.0f}, .fLeftLeg = {0.0f, 38.0f, 353.0f}, .fRightLeg = {3.0f, 243.0f, 0.0f}, .fLeftArm = {16.0f, 0.0f, 0.0f}, .fRightArm = {248.0f, 38.0f, 0.0f}},
    };
    return std::ref(sPoses);
  }

  static std::optional<Pose> JavaPoseFromBedrockPoseIndex(i32 poseIndex) {
    auto const &poses = GetJavaPoses();
    if (0 <= poseIndex && poseIndex < poses.size()) {
      return poses[poseIndex];
    } else {
      return std::nullopt;
    }
  }

  static float RotSimilarity(Rot a, Rot b) {
    using namespace std;
    return cosf(get<0>(a) - get<0>(b)) + cosf(get<1>(a) - get<1>(b)) + cosf(get<2>(a) - get<2>(b));
  }

  static float PoseSimilarity(Pose a, Pose b) {
    return RotSimilarity(a.fBody, b.fBody)           //
           + RotSimilarity(a.fHead, b.fHead)         //
           + RotSimilarity(a.fLeftLeg, b.fLeftLeg)   //
           + RotSimilarity(a.fRightLeg, b.fRightLeg) //
           + RotSimilarity(a.fLeftArm, b.fLeftArm)   //
           + RotSimilarity(a.fRightArm, b.fRightArm);
  }

  static std::optional<i32> BedrockMostSimilarPoseIndexFromJava(CompoundTag const &javaPose) {
    Pose pose = Pose::FromCompoundTag(javaPose);
    float maxSimilarity = std::numeric_limits<float>::lowest();
    int index = -1;
    auto const &poses = GetJavaPoses();
    for (int i = 0; i < poses.size(); i++) {
      float similarity = PoseSimilarity(pose, poses[i]);
      if (similarity > maxSimilarity) {
        index = i;
        maxSimilarity = similarity;
      }
    }
    if (index >= 0) {
      return index;
    } else {
      return std::nullopt;
    }
  }
};

} // namespace je2be
