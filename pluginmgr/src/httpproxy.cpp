#include <httpproxy.h>
#include <yjson.h>

#ifdef _WIN32
#include <systemapi.h>
#endif

static const wchar_t regProxyPath[] = LR"(Software\Microsoft\Windows\CurrentVersion\Internet Settings)";

HttpProxy::HttpProxy(YJson& settings)
  : m_Settings(InitSettings(settings))
  , m_Proxy(settings[u8"Proxy"].getValueString())
  , m_Username(settings[u8"Username"].getValueString())
  , m_Password(settings[u8"Password"].getValueString())
  , m_Type(settings[u8"Type"].getValueDouble())
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

HttpProxy::String HttpProxy::GetProxy()
{
  Locker locker(m_Mutex);
  return m_Proxy;
}

void HttpProxy::SetProxy(String proxy)
{
  Locker locker(m_Mutex);
  m_Proxy.swap(proxy);
}

HttpProxy::String HttpProxy::GetUsername()
{
  Locker locker(m_Mutex);
  return m_Username;
}

void HttpProxy::SetUsername(String username)
{
  Locker locker(m_Mutex);
  m_Username.swap(username);
}

HttpProxy::String HttpProxy::GetPassword()
{
  Locker locker(m_Mutex);
  return m_Password;
}

void HttpProxy::SetPassword(String password)
{
  Locker locker(m_Mutex);
  m_Password.swap(password);
}
int HttpProxy::GetType()
{
  Locker locker(m_Mutex);
  return static_cast<int>(m_Type);
}

void HttpProxy::SetType(int type)
{
  Locker locker(m_Mutex);
  m_Type = type;
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