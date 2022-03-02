#pragma once

namespace je2be::tobe {

class PlayerAbilities {
public:
  bool fAttackMobs = true;
  bool fAttackPlayers = true;
  bool fBuild = true;
  bool fDoorsAndSwitches = true;
  bool fFlying = false;
  float fFlySpeed = 0.05f;
  bool fInstabuild = false;
  bool fInvulnerable = false;
  bool fLightning = false;
  bool fMayFly = false;
  bool fMine = true;
  bool fOp = false;
  bool fOpenContainers = true;
  int fPermissionsLevel = 0;
  int fPlayerPermissionsLevel = 1;
  bool fTeleport = false;
  float fWalkSpeed = 0.1f;

  CompoundTagPtr toCompoundTag() const {
    auto a = Compound();
    a->insert({
        {"attackmobs", Bool(fAttackMobs)},
        {"attackplayers", Bool(fAttackPlayers)},
        {"build", Bool(fBuild)},
        {"doorsandswitches", Bool(fDoorsAndSwitches)},
        {"flying", Bool(fFlying)},
        {"flySpeed", Float(fFlySpeed)},
        {"instabuild", Bool(fInstabuild)},
        {"invulnerable", Bool(fInvulnerable)},
        {"lightning", Bool(fLightning)},
        {"mayfly", Bool(fMayFly)},
        {"mine", Bool(fMine)},
        {"op", Bool(fOp)},
        {"opencontainers", Bool(fOpenContainers)},
        {"permissionsLevel", Int(fPermissionsLevel)},
        {"playerPermissionsLevel", Int(fPlayerPermissionsLevel)},
        {"teleport", Bool(fTeleport)},
        {"walkSpeed", Float(fWalkSpeed)},
    });
    return a;
  }

  static PlayerAbilities Import(CompoundTag const &tag) {
    PlayerAbilities ret;
    ret.fFlying = tag.boolean("flying", ret.fFlying);
    ret.fFlySpeed = tag.float32("flySpeed", ret.fFlySpeed);
    ret.fInstabuild = tag.boolean("instabuild", ret.fInstabuild);
    ret.fInvulnerable = tag.boolean("invulnerable", ret.fInvulnerable);
    ret.fBuild = tag.boolean("mayBuild", ret.fBuild);
    ret.fMayFly = tag.boolean("mayfly", ret.fMayFly);
    ret.fWalkSpeed = tag.float32("walkSpeed", ret.fWalkSpeed);
    return ret;
  }
};

} // namespace je2be::tobe
