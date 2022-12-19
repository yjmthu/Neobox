#ifdef CLASS_NAME

#include <pluginobject.h>
#include <neoapp.h>
#include <pluginmgr.h>
#include <yjson.h>

GlbObject *glb;
PluginMgr *mgr;

extern "C" _declspec(dllexport) PluginObject* newPlugin(YJson& settings, PluginMgr* _mgr)
{
  mgr = _mgr;
  glb = mgr->m_GlbObject;
  return new CLASS_NAME(settings);
}

#undef CLASS_NAME

#endif
