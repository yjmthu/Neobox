#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <filesystem>
#include <string>
#include <map>
#include <httpproxy.h>

class HttpLib {
private:
  struct PostData { void* data; size_t size; } m_PostData;
  typedef HttpProxy::String String;
public:
  typedef std::map<std::string, std::string> Headers;
  typedef size_t( CallbackFunction )(void*, size_t, size_t, void*);
  typedef CallbackFunction *pCallbackFunction;
  struct Response {
    std::string version;
    unsigned long status = -1;
    std::string reason;
    Headers headers;
    std::string body;
    std::string location; // Redirect location
  };
  template<typename Char=char>
  explicit HttpLib(std::basic_string_view<Char> url):
    m_Url(url.cbegin(), url.cend()),
    m_hSession(nullptr),
    m_ProxySet(false)
  {
    HttpInit();
  }
  template<typename Char=char>
  explicit HttpLib(std::basic_string<Char> url)
    : HttpLib(std::basic_string_view<Char>(url))
  {
  }
  ~HttpLib();
public:
  template<typename Char=char>
  void SetUrl(std::basic_string_view<Char> url) {
    m_Url.assign(url.begin(), url.end());
    HttpInit();
  }
  template<typename Char=char>
  void SetUrl(const std::basic_string<Char>& url) {
    SetUrl(std::basic_string_view<Char>(url));
  }
  void SetHeader(std::string key, std::string value) {
    m_Headers[key] = value;
  }
  void SetRedirect(long redirect);
  void SetPostData(void* data, size_t size);
  Response* Get();
  Response* Get(const std::filesystem::path& path);
  Response* Get(pCallbackFunction callback, void* userData);
  void Exit();
  static bool IsOnline();
public:
  static std::optional<HttpProxy> m_Proxy;
private:
  std::atomic_bool m_Exit = false;
  Headers m_Headers;
  Response m_Response;
  std::u8string m_Url;
  void* m_hSession = nullptr;
#ifdef _WIN32
  void* m_hConnect = nullptr;
  void* m_hRequest = nullptr;
#endif
  int m_TimeOut = 30;
  bool m_ProxySet;
private:
  void HttpInit();
  void HttpPerform();
  void SendHeaders();
  void SetProxyBefore();
  void SetProxyAfter();
  bool SendRequestData();
  String GetDomain();
  String GetPath();
private:
  static CallbackFunction WriteFile;
  static CallbackFunction WriteString;
  pCallbackFunction m_CallBack;
  void* m_DataBuffer;
};

#endif
