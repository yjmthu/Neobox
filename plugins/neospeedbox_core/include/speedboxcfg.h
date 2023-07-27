#ifndef SPEEDBOXCFG_H
#define SPEEDBOXCFG_H

#include <neoconfig.h>

class SpeedBoxCfg: public NeoConfig {
  ConfigConsruct(SpeedBoxCfg)
  CfgBool(ShowForm)
  CfgArray(Position)
  CfgInt(HideAside)
  CfgBool(ColorEffect)
  CfgArray(BackgroundColorRgba)
  CfgString(CurSkin)
  CfgObject(UserSkins)
  CfgYJson(NetCardDisabled)
  CfgBool(MousePenetrate)
  CfgBool(ProgressMonitor)
  CfgBool(TaskbarMode)
};

#endif // SPEEDBOXCFG_H