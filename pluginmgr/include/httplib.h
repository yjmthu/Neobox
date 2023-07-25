#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <filesystem>
#include <string>
#include <map>
#include <httpproxy.h>

class HttpLib {
private:
  struct PostData { void* data; size_t size; } m_PostData;
public:
  typedef std::map<std::string, std::string> Headers;
  typedef size_t( CallbackFunction )(void*, size_t, size_t, void*);
  typedef CallbackFunction *pCallbackFunction;
  typedef std::function<void(const void*, size_t)> WriteCallback;
  struct Response {
    std::string version;
    unsigned long status = -1;
    std::string reason;
    Headers headers;
    std::string body;
    std::string location; // Redirect location
  };
  typedef std::function<void(std::wstring, const Response*)> ErrorCallback;
  template<typename Char=char>
  explicit HttpLib(std::basic_string_view<Char> url, bool async=false):
    m_Url(url.cbegin(), url.cend()),
    m_hSession(nullptr),
    m_ProxySet(false),
    m_AsyncSet(async)
  {
    HttpInit();
  }
  template<typename Char=char>
  explicit HttpLib(std::basic_string<Char> url, bool async=false)
    : HttpLib(std::basic_string_view<Char>(url), async)
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
  void GetAsync(WriteCallback writeData, ErrorCallback error);
  void Exit();
  static bool IsOnline();
public:
  static std::optional<HttpProxy> m_Proxy;
  bool SetAsyncCallback();
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
  int m_RedirectDepth = 0;
  bool m_ProxySet;
  bool m_AsyncSet;
  std::function<void()> m_AsyncFunc;
private:
  void HttpInit();
  void HttpPerform();
  bool SendHeaders();
  void SetProxyBefore();
  bool SetProxyAfter();
  bool SendRequest();
  bool RecvResponse();
  std::u8string GetDomain();
  std::u8string GetPath();
private:
  bool ReadStatusCode();
  bool ReadHeaders();
  bool ReadBody();
private:
  static CallbackFunction WriteFile;
  static CallbackFunction WriteString;
  static void RequestStatusCallback(void* hInternet, unsigned long long dwContext, unsigned long dwInternetStatus, void* lpvStatusInformation, unsigned long dwInternetInformationLength);
  pCallbackFunction m_CallBack;
  WriteCallback m_AsyncWrite;
  ErrorCallback m_AsyncError;
  void* m_DataBuffer;
};

#endif
