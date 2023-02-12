#ifdef CLASS_NAME

#include <pluginobject.h>
#include <pluginmgr.h>
#include <yjson.h>

PluginMgr *mgr;

extern "C"
#ifdef _WIN32
_declspec(dllexport)
#endif
PluginObject* newPlugin(YJson& settings, PluginMgr* _mgr)
{
  mgr = _mgr;
  return new CLASS_NAME(settings);
}

#undef CLASS_NAME

#endif
