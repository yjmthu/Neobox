#ifndef USBCONFIG_H
#define USBCONFIG_H

#include <neoconfig.h>

class UsbConfig: public NeoConfig
{
  ConfigConsruct(UsbConfig)
  CfgBool(StayOnTop)
  CfgBool(HideWhenFull)
  CfgBool(HideAside)
  CfgYJson(Position)
};

#endif // USBCONFIG_H