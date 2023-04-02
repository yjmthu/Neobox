#ifndef HTTPPROXY_H
#define HTTPPROXY_H

#include <neoconfig.h>
#include <string>

class HttpProxy: public NeoConfig
{
public:
  explicit HttpProxy(YJson& settings);

  static std::u8string GetSystemProxy();
  static bool IsSystemProxy();
  bool IsUserEmpty();
  void UpdateSystemProxy();
private:
  static YJson& InitSettings(YJson&);
  CfgInt(Type)
  CfgString(Proxy)
  CfgString(Username)
  CfgString(Password)
};

#endif