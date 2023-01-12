#ifdef CLASS_NAME

#include <pluginobject.h>
#include <pluginmgr.h>
#include <yjson.h>

PluginMgr *mgr;

extern "C" _declspec(dllexport) PluginObject* newPlugin(YJson& settings, PluginMgr* _mgr)
{
  mgr = _mgr;
  return new CLASS_NAME(settings);
}

#undef CLASS_NAME

#endif
