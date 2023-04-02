#ifndef OCRCONFIG_H
#define OCRCONFIG_H

#include <neoconfig.h>

class OcrConfig: public NeoConfig
{
  ConfigConsruct(OcrConfig)
  CfgString(TessdataDir)
  CfgString(WinLan)
  CfgArray(Languages)
  CfgBool(WriteClipboard)
  CfgBool(ShowWindow)
  CfgInt(OcrEngine)
};

#endif // OCRCONFIG_H