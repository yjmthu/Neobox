#ifdef PluginName

#include <neobox/pluginobject.h>
#include <neobox/pluginmgr.h>
#include <yjson/yjson.h>

PluginMgr *mgr;

extern "C"
#ifdef _WIN32
_declspec(dllexport)
#endif
PluginObject* newPlugin(YJson& settings, PluginMgr* _mgr)
{
  mgr = _mgr;
  return new PluginName(settings);
}

#endif
