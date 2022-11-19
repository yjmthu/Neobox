#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <filesystem>
#include <string>
#include <map>

class HttpLib {
public:
  typedef std::map<std::string, std::string> Headers;
  typedef size_t( CallbackFunction )(void*, size_t, size_t, void*);
  typedef CallbackFunction *pCallbackFunction;
  struct Response {
    std::string version;
    long status = -1;
    std::string reason;
    Headers headers;
    std::string body;
    std::string location; // Redirect location
  };
  template<typename Char=char>
  explicit HttpLib(std::basic_string<Char> url):
    m_Url(url.cbegin(), url.cend()),
    m_Curl(nullptr)
  {
    m_Url.push_back('\0');
    CurlInit();
  }
  ~HttpLib();
  static bool IsOnline();
  template<typename Char=char>
  void SetUrl(std::basic_string<Char> url) {
    m_Url = std::move(url);
    m_Url.push_back('\0');
    CurlInit();
  }
  void SetHeader(std::string key, std::string value) {
    m_Headers[key] = value;
  }
  void SetRedirect(long redirect);
  Response* Get();
  Response* Get(const std::filesystem::path& path);
private:
  Headers m_Headers;
  Response m_Response;
  std::string m_Url;
  void* m_Curl;

  void CurlInit();
  void CurlPerform();
  static CallbackFunction WriteFile;
  static CallbackFunction WriteString;
};

#endif
