#ifndef COLORCONFIG_H
#define COLORCONFIG_H

#include <neoconfig.h>

class ColorConfig: public NeoConfig
{
  ConfigConsruct(ColorConfig)
  CfgBool(StayTop)
  CfgInt(HistoryMaxCount)
  CfgArray(History)
};

#endif // COLORCONFIG_H