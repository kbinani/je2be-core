#pragma once

namespace je2be::tobe {

class PlayerAbilities {
public:
  // NOTE: Default values are for the "Member" permission
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
  bool fTeleport = false;
  float fWalkSpeed = 0.1f;

  CompoundTagPtr toCompoundTag() const {
    auto a = Compound();
    a->insert({
        {u8"attackmobs", Bool(fAttackMobs)},
        {u8"attackplayers", Bool(fAttackPlayers)},
        {u8"build", Bool(fBuild)},
        {u8"doorsandswitches", Bool(fDoorsAndSwitches)},
        {u8"flying", Bool(fFlying)},
        {u8"flySpeed", Float(fFlySpeed)},
        {u8"instabuild", Bool(fInstabuild)},
        {u8"invulnerable", Bool(fInvulnerable)},
        {u8"lightning", Bool(fLightning)},
        {u8"mayfly", Bool(fMayFly)},
        {u8"mine", Bool(fMine)},
        {u8"op", Bool(fOp)},
        {u8"opencontainers", Bool(fOpenContainers)},
        {u8"teleport", Bool(fTeleport)},
        {u8"walkSpeed", Float(fWalkSpeed)},
    });
    return a;
  }

  static PlayerAbilities Import(CompoundTag const &tag) {
    PlayerAbilities ret;
    ret.fFlying = tag.boolean(u8"flying", ret.fFlying);
    ret.fFlySpeed = tag.float32(u8"flySpeed", ret.fFlySpeed);
    ret.fInstabuild = tag.boolean(u8"instabuild", ret.fInstabuild);
    ret.fInvulnerable = tag.boolean(u8"invulnerable", ret.fInvulnerable);
    ret.fBuild = tag.boolean(u8"mayBuild", ret.fBuild);
    ret.fMayFly = tag.boolean(u8"mayfly", ret.fMayFly);
    ret.fWalkSpeed = tag.float32(u8"walkSpeed", ret.fWalkSpeed);
    return ret;
  }
};

} // namespace je2be::tobe
