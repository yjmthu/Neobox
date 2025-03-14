#ifndef HTTPPROXY_H
#define HTTPPROXY_H

#include <neobox/neoconfig.h>
#include <string>

class HttpProxy: public NeoConfig
{
public:
  enum class Type { System = 0, User = 1, None = 2 };

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