#ifndef TRANSLATECFG_H
#define TRANSLATECFG_H

#include <neoconfig.h>

class TranslateCfg: public NeoConfig
{
  ConfigConsruct(TranslateCfg)
  CfgInt(Mode)
  CfgArray(Size)
  CfgArray(PairBaidu)
  CfgArray(PairYoudao)
  CfgArray(Position)
  CfgYJson(HeightRatio)
  CfgBool(AutoTranslate)
  CfgBool(ReadClipboard)
  CfgBool(AutoMove)
  CfgBool(AutoSize)
};

#endif // TRANSLATECFG_H
