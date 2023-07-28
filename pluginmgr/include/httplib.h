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

  struct Response {
    std::string version;
    unsigned long status = -1;
    std::string reason;
    Headers headers;
    std::string body;
    std::string location; // Redirect location
  };

  typedef std::function<void(const void*, size_t)> WriteCallback;
  typedef std::function<void(size_t, size_t)> ProcessCallback;
  typedef std::function<void(std::wstring, const Response*)> FinishCallback;
  struct Callback {
    std::optional<WriteCallback> m_WriteCallback;
    FinishCallback m_FinishCallback;
    std::optional<ProcessCallback> m_ProcessCallback;
  };

  template<typename Char=char>
  explicit HttpLib(std::basic_string_view<Char> url, bool async=false):
    m_Url(url.cbegin(), url.cend()),
    m_hSession(nullptr),
    m_ProxySet(false),
    m_AsyncSet(async) {
    HttpPrepare();
    HttpInitialize();
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
    HttpUninitialize();
    HttpInitialize();
  }
  template<typename Char=char>
  void SetUrl(const std::basic_string<Char>& url) {
    SetUrl(std::basic_string_view<Char>(url));
  }
  void SetHeader(std::string key, std::string value) {
    m_Headers[key] = value;
  }
  void ClearHeader() {
    m_Headers.clear();
  }
  void SetRedirect(long redirect);
  void SetPostData(void* data, size_t size);
  Response* Get();
  Response* Get(const std::filesystem::path& path);
  Response* Get(CallbackFunction* callback, void* userData);
  void GetAsync(Callback callback);
  void ExitAsync();
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
  int m_RedirectDepth = 0;
  bool m_ProxySet;
  bool m_AsyncSet;
  bool m_AsyncFinished;
  size_t m_RecieveSize = 0;
  size_t m_ConnectLength = 0;
private:
  void HttpInitialize();
  void HttpUninitialize();
  void HttpPrepare();
  void HttpPerform();
  void ResetData();
  bool SendHeaders();
  void SetProxyBefore();
  bool SetProxyAfter();
  bool SetAsyncCallback();
  bool SendRequest();
  bool RecvResponse();
  std::u8string GetDomain();
  std::u8string GetPath();
private:
  bool ReadStatusCode();
  bool ReadHeaders();
  bool ReadBody();
  void EmitProcess();
  void EmitFinish(std::wstring message=L"");
private:
  static CallbackFunction WriteFile;
  static CallbackFunction WriteString;
  static void RequestStatusCallback(void* hInternet, unsigned long long dwContext, unsigned long dwInternetStatus, void* lpvStatusInformation, unsigned long dwInternetInformationLength);
  CallbackFunction* m_Callback;
  Callback m_AsyncCallback;
  void* m_DataBuffer;
};

#endif
