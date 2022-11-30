#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <filesystem>
#include <string>
#include <map>

class HttpLib {
private:
  struct PostData { void* data; size_t size; } m_PostData;
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
  explicit HttpLib(std::basic_string<Char> url):
    m_Url(url.cbegin(), url.cend()),
    m_hSession(nullptr)
  {
    HttpInit();
  }
  ~HttpLib();
public:
  template<typename Char=char>
  void SetUrl(std::basic_string<Char> url) {
    m_Url = std::move(url);
    HttpInit();
  }
  void SetHeader(std::string key, std::string value) {
    m_Headers[key] = value;
  }
  void SetRedirect(long redirect);
  void SetPostData(void* data, size_t size);
  Response* Get();
  Response* Get(const std::filesystem::path& path);
  Response* Get(CallbackFunction *callback, void* userData);
  static bool IsOnline();
private:
  Headers m_Headers;
  Response m_Response;
  std::string m_Url;
  void* m_hSession = nullptr;
#ifdef _WIN32
  void* m_hConnect = nullptr;
  void* m_hRequest = nullptr;
#endif
private:
  void HttpInit();
  void HttpPerform();
  void SendHeaders();
  bool SendRequestData();
  std::wstring GetDomain();
  std::wstring GetPath();
private:
  static CallbackFunction WriteFile;
  static CallbackFunction WriteString;
  CallbackFunction* m_CallBack;
  void* m_DataBuffer;
};

#endif
