#ifndef WALLCONFIG_H
#define WALLCONFIG_H

#include <neoconfig.h>

struct WallConfig: NeoConfig
{
  ConfigConsruct(WallConfig)
  CfgInt(ImageType)
  CfgInt(TimeInterval)
  CfgBool(AutoChange)
  CfgBool(FirstChange)
  CfgBool(DropImgUseUrlName)
  CfgString(DropDir)
  CfgString(DropNameFmt)
};

#endif // WALLCONFIG_H
