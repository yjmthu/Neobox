#ifndef _DEBUG
#ifdef CLASS_NAME

#include <pluginobject.h>
#include <yjson.h>

extern "C" _declspec(dllexport) PluginObject* newPlugin(YJson& settings);

PluginObject* newPlugin(YJson& settings)
{
  return new CLASS_NAME(settings);
}

#undef CLASS_NAME

#endif
#endif