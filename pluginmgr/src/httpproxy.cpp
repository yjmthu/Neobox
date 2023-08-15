#include <neobox/httpproxy.h>
#include <yjson/yjson.h>

#ifdef _WIN32
#include <neobox/systemapi.h>
static const wchar_t regProxyPath[] = LR"(Software\Microsoft\Windows\CurrentVersion\Internet Settings)";
#endif

HttpProxy::HttpProxy(YJson& settings)
  : NeoConfig(InitSettings(settings))
{}

YJson& HttpProxy::InitSettings(YJson& settings)
{
  if (settings.isNull()) {
    settings = YJson::O {
      { u8"Type", YJson::Number },
      { u8"Proxy", GetSystemProxy() },
      { u8"Username", YJson::String },
      { u8"Password", YJson::String }
    };
  }
  auto& proxy = settings[u8"Proxy"];
  if (!proxy.isString()) {
    proxy = GetSystemProxy();
  }
  return settings;
}

std::u8string HttpProxy::GetSystemProxy()
{
#ifdef _WIN32
  return Wide2Utf8String(RegReadString(HKEY_CURRENT_USER, regProxyPath, L"ProxyServer"));
#else
  auto const str = reinterpret_cast<const char8_t*>(std::getenv("HTTP_PROXY"));
  if (!str) return {};
  return str;
#endif
}

bool HttpProxy::IsSystemProxy()
{
#ifdef _WIN32
  return RegReadValue(HKEY_CURRENT_USER, regProxyPath, L"ProxyEnable");
#else
  auto const str = reinterpret_cast<const char8_t*>(std::getenv("HTTP_PROXY"));
  return str;
#endif
}

bool HttpProxy::IsUserEmpty()
{
  Locker locker(m_Mutex);
  return m_Username.empty() || m_Password.empty();
}

void HttpProxy::UpdateSystemProxy()
{
  Locker locker(m_Mutex);
  m_Username.clear();
  m_Password.clear();
  m_Proxy = GetSystemProxy();
}