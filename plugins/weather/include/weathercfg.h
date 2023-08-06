#ifndef WEATHERCFG_H
#define WEATHERCFG_H

#include <neoconfig.h>

class WeatherCfg: NeoConfig {
  ConfigConsruct(WeatherCfg)
  CfgString(ApiKey)
  CfgBool(Prompt)
  CfgBool(IsPaidUser)
  CfgYJson(CityList)
  CfgString(City)
  CfgString(UpdateCycle)
  CfgArray(WindowPosition)
};

#endif // WEATHERCFG_H