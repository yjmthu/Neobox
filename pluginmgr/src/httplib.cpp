#ifdef _WIN32
#include <systemapi.h>
#include <Shlobj.h>
#include <winhttp.h>
#else
#include <curl/curl.h>
#include <curl/header.h>
#endif  // _WIN32

#include <httplib.h>
#include <filesystem>
#include <stdexcept>
#include <fstream>
#include <format>

using namespace std::literals;
namespace fs = std::filesystem;

HttpProxy HttpLib::m_Proxy;
static const wchar_t regProxyPath[] = LR"(Software\Microsoft\Windows\CurrentVersion\Internet Settings)";

void HttpProxy::GetSystemProxy()
{
  username.clear();
  password.clear();
  domain = RegReadString(HKEY_CURRENT_USER, regProxyPath, L"ProxyServer");
  auto const pos = domain.find(L':');
  if (pos == domain.npos) {
    port = 1;
  } else {
    port = std::stoi(domain.substr(pos + 1));
    domain.erase(pos);
  }
}

bool HttpLib::IsOnline() {
#ifdef _WIN32
  BOOL bResult = FALSE;
  DWORD flags;
  typedef BOOL(* pInternetGetConnectedState)(LPDWORD, DWORD*);
  
  HMODULE hWininet = LoadLibraryW(L"Wininet.dll");
  if (hWininet) {
    pInternetGetConnectedState InternetGetConnectedState =
        (pInternetGetConnectedState)GetProcAddress(
            hWininet, "InternetGetConnectedState");
    if (InternetGetConnectedState) {
      bResult = InternetGetConnectedState(&flags, 0);
    }
    FreeLibrary(hWininet);
  } else {
    throw std::runtime_error("Can't find Wininet library!");
  }
  return bResult;
#elif 0
  std::vector<std::string> result;
  GetCmdOutput<char>("ping www.baidu.com -c 2", result);
  if (result.size() < 2)
    return false;
  auto& data = result.end()[-2];
  auto first = data.find("received");
  if (first == std::string::npos)
    return false;
  first += 10;
  auto last = data.find("%", first);
  auto&& lostPacket = data.substr(first, last - first);
  std::cout << data << std::endl << "lostPacket: " << lostPacket << std::endl;
  return !std::atoi(lostPacket.c_str());
#else
  httplib::Client clt("https://www.baidu.com");
  auto res = clt.Get("/");
  return res && res->status == 200;
#endif
}

size_t HttpLib::WriteFile(void* buffer,
                        size_t size,
                        size_t nmemb,
                        void* userdata) {
  std::ofstream& stream = *reinterpret_cast<std::ofstream*>(userdata);
  stream.write(reinterpret_cast<const char*>(buffer), (size *= nmemb));
  return size;
}

size_t HttpLib::WriteString(void* buffer,
                          size_t size,
                          size_t nmemb,
                          void* userdata) {
  const char *first = reinterpret_cast<const char*>(buffer),
                *last = first + (size *= nmemb);
  std::string& str = *reinterpret_cast<std::string*>(userdata);
  str.append(first, last);
  return size;
}

HttpLib::~HttpLib() {
#ifdef _WIN32
  if(m_hRequest)
    WinHttpCloseHandle(m_hRequest);
  if (m_hConnect)
    WinHttpCloseHandle(m_hConnect);
  if (m_hSession)
    WinHttpCloseHandle(m_hSession);
#else
  if (m_hSession)
    curl_easy_cleanup(m_hSession);
#endif
}

std::wstring HttpLib::GetDomain()
{
  std::wstring result;
  if (m_Url.starts_with("https://")) {
    result = Ansi2WideString(m_Url.substr(8, m_Url.find('/', 8) - 8));
  } else if (m_Url.starts_with("http://")) {
    result = Ansi2WideString(m_Url.substr(7, m_Url.find('/', 7) - 7));
  } else {
    throw std::logic_error("Url should begin with 'http://' or 'http://'!");
  }
  result.push_back(L'\0');
  return result;
}

std::wstring HttpLib::GetPath()
{
  size_t pos;
  if (m_Url.starts_with("https://")) {
    pos = m_Url.find('/', 8);
  } else if (m_Url.starts_with("http://")) {
    pos = m_Url.find('/', 7);
  } else {
    throw std::logic_error("Url should begin with 'http://' or 'http://'!");
  }
  if (pos == std::string::npos) {
    return L"/"s;
  }
  auto result = Ansi2WideString(m_Url.substr(pos));
  result.push_back(L'\0');
  return result;
}

void HttpLib::HttpInit()
{
  if (!WinHttpCheckPlatform()) {
    throw std::runtime_error("This platform is NOT supported by WinHTTP.");
  }
  m_PostData.data = nullptr;
  m_PostData.size = 0;
#ifdef _WIN32
  if (m_hConnect) WinHttpCloseHandle(m_hConnect);
  if (m_hSession) {
    WinHttpCloseHandle(m_hSession);
  }
  m_hSession = WinHttpOpen(L"WinHTTP in Neobox/1.0", 
    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
    WINHTTP_NO_PROXY_NAME, 
    WINHTTP_NO_PROXY_BYPASS, 0);
  if (m_hSession) {
    auto url = GetDomain();
    m_hConnect = WinHttpConnect(m_hSession, url.data(), INTERNET_DEFAULT_HTTP_PORT, 0);
    SetProxyBefore();
  }

#else
  m_Url.push_back('\0');
  if (m_hSession)
    curl_easy_cleanup(m_hSession);
  m_hSession = curl_easy_init();
  curl_easy_setopt(m_hSession, CURLOPT_HEADER, false);
  curl_easy_setopt(m_hSession, CURLOPT_URL, m_Url.data());
  curl_easy_setopt(m_hSession, CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(m_hSession, CURLOPT_SSL_VERIFYHOST, false);
  curl_easy_setopt(m_hSession, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(m_hSession, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(m_hSession, CURLOPT_POST, 0L);
#endif
}

void HttpLib::SetProxyBefore()
{
  if (m_Proxy.type == 3) return;
  if (m_Proxy.type == 0) {
    if (!RegReadValue(HKEY_CURRENT_USER, regProxyPath, L"ProxyEnable")) return;
    m_Proxy.GetSystemProxy();
  }
  std::wstring proxyString = std::format(L"{}:{}", m_Proxy.domain, m_Proxy.port);
  proxyString.push_back(L'\0');
  // if (proxyString.starts_with(L"http://")) {
  //   proxyString = L"http://" + proxyString;
  // }

  WINHTTP_PROXY_INFO proxy { 
    WINHTTP_ACCESS_TYPE_NAMED_PROXY,
    proxyString.data(), nullptr
  };

  if (WinHttpSetOption(m_hSession, WINHTTP_OPTION_PROXY, &proxy, sizeof(proxy)))
  {
    if (m_Proxy.password.empty() || m_Proxy.username.empty()) return;
    m_ProxySet = true;
  }
}

void HttpLib::SetProxyAfter()
{
  if (!m_ProxySet) return;
  WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_USERNAME, m_Proxy.username.data(), m_Proxy.username.size() * sizeof(wchar_t));
  WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_PASSWORD, m_Proxy.password.data(), m_Proxy.password.size() * sizeof(wchar_t));
}

void HttpLib::SetRedirect(long redirect)
{
#ifdef _WIN32
  ULONGLONG flag = redirect ?
    WINHTTP_OPTION_REDIRECT_POLICY_DISALLOW_HTTPS_TO_HTTP : WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
  WinHttpSetOption(m_hSession, WINHTTP_OPTION_REDIRECT_POLICY, &flag, sizeof(flag));
#else
  curl_easy_setopt(m_hSession, CURLOPT_FOLLOWLOCATION, redirect);
#endif
}

void HttpLib::SetPostData(void *data, size_t size)
{
  m_PostData.data = data;
  m_PostData.size = size;
}

// https://blog.csdn.net/kaola518/article/details/84065621
bool HttpLib::SendRequestData()
{
  if (m_PostData.data) {
#ifdef _WIN32
    if (!m_hRequest) return false;
    return WinHttpSendRequest(m_hRequest,
      WINHTTP_NO_ADDITIONAL_HEADERS, 0, m_PostData.data, m_PostData.size, m_PostData.size, 0);
#else
    curl_easy_setopt(m_hSession, CURLOPT_POST, 1L);
    curl_easy_setopt(m_hSession, CURLOPT_POSTFIELDS, m_PostData.data);
    curl_easy_setopt(m_hSession, CURLOPT_POSTFIELDSIZE, m_PostData.size);
#endif
  } else {
#ifdef _WIN32
    if (!m_hRequest) return false;
    return WinHttpSendRequest(m_hRequest,
      WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
#endif
  }
  return true;
}

void HttpLib::SendHeaders()
{
#ifdef _WIN32
  if(m_hRequest) {
    for (auto& [i, j]: m_Headers) {
      auto header = Ansi2WideString(std::format("{}: {}", i, j));
      WinHttpAddRequestHeaders(m_hRequest, header.data(), header.size(), WINHTTP_ADDREQ_FLAG_ADD);
    }
  }
#else
  if (!m_Headers.empty()) {
    struct curl_slist *headers = nullptr;
    std::string buffer;
    for (const auto& [key, value]: m_Headers) {
      buffer = std::format("{}: {}", key, value);
      buffer.push_back('\0');
      headers = curl_slist_append(headers, buffer.data());
    }
    curl_easy_setopt(m_hSession, CURLOPT_HTTPHEADER, headers);
  }
#endif
}

void HttpLib::HttpPerform()
{
  bool bResults = false;
#ifdef _WIN32
  auto path = GetPath();
  m_hRequest = WinHttpOpenRequest (m_hConnect,
    m_PostData.data ? L"POST": L"GET",
    path.data(), L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
  if (m_hRequest) {
    SetProxyAfter();
  }
#else
  curl_easy_setopt(m_hSession, CURLOPT_WRITEFUNCTION, m_CallBack);
  curl_easy_setopt(m_hSession, CURLOPT_WRITEDATA, m_DataBuffer);
#endif
  SendHeaders();
#ifdef _WIN32
  if (m_hRequest) {
    bResults = SendRequestData();
  } else {
    std::cerr << "WinHttpOpenRequest failed: " << GetLastError() << std::endl;
  }
  if(bResults) {
    bResults = WinHttpReceiveResponse(m_hRequest, NULL);
  } else {
    std::cerr << "WinHttpSendRequest failed: " << GetLastError() << std::endl;
  }
#else
  auto lStatus = curl_easy_perform(m_hSession);
  bResults = lStatus == CURLE_OK;
#endif
#ifdef _WIN32
  if(DWORD dwSize = sizeof(m_Response.status); bResults) 
    bResults = WinHttpQueryHeaders(m_hRequest, 
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
      NULL, &m_Response.status, &dwSize, WINHTTP_NO_HEADER_INDEX);
  else
    std::cerr << "WinHttpReceiveResponse failed: " << GetLastError() << std::endl;
  if(bResults)
  {
    if(m_Response.status == 304) 
      std::cout << "Document has not been updated.\n";
  }
#else
  if (bResults)
    lStatus = curl_easy_getinfo(m_hSession, CURLINFO_RESPONSE_CODE, &m_Response.status);
  else
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(lStatus) << std::endl;
  bResults = lStatus == CURLE_OK;
#endif
  if(bResults) {
    m_Response.headers.clear();
#ifdef _WIN32
    DWORD dwSize = 0;
    WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                          WINHTTP_HEADER_NAME_BY_INDEX, NULL,
                          &dwSize, WINHTTP_NO_HEADER_INDEX);

    // Allocate memory for the buffer.
    if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
      auto const lpOutBuffer = new WCHAR[dwSize/sizeof(WCHAR)];

      // Now, use WinHttpQueryHeaders to retrieve the header.
      bResults = WinHttpQueryHeaders(m_hRequest,
                                  WINHTTP_QUERY_RAW_HEADERS_CRLF,
                                  WINHTTP_HEADER_NAME_BY_INDEX,
                                  lpOutBuffer, &dwSize,
                                  WINHTTP_NO_HEADER_INDEX);

      if (bResults) { // regex expr: '/^([^:]+):([^\n]+)/'
        std::istringstream strstream(Wide2AnsiString(lpOutBuffer));
        std::string buffer;
        if (std::getline(strstream, buffer)) {
          auto const pos = buffer.find(' ');
          m_Response.version = buffer.substr(0, pos);
        }
        while (std::getline(strstream, buffer)) {
          // const auto str = Wide2AnsiString(buffer);
          const auto left = buffer.find(':'), right = buffer.find('\r', left);
          if (left == buffer.npos || right == buffer.npos) continue;
          m_Response.headers[buffer.substr(0, left)] = buffer.substr(left + 1, right - left - 1);
        }
      }

      delete[] lpOutBuffer;
    }

    if (bResults) {
      std::string strBuffer;
      for (DWORD dwDownloaded = 0;;) {
        bResults = WinHttpQueryDataAvailable(m_hRequest, &dwSize);
        if (!bResults) {
          std::wcerr << L"WinHttpQueryDataAvailable failed: " << GetLastError() << std::endl;
          break;
        }           
        if (dwSize == 0) break;
        strBuffer.resize(dwSize);
        bResults = WinHttpReadData(m_hRequest, strBuffer.data(), dwSize, &dwDownloaded);
        if (!bResults) {
          std::wcout << L"WinHttpQueryDataAvailable failed: " << GetLastError() << std::endl;
        }
        if (!dwDownloaded) break;
        m_CallBack(strBuffer.data(), sizeof(char), dwDownloaded, m_DataBuffer);
      };
    }
#else
    char *ct = nullptr;
    lStatus = curl_easy_getinfo(m_hSession, CURLINFO_CONTENT_TYPE, &ct);
    if(!lStatus && ct) {
      m_Response.headers["Content-Type"] = ct;
    }
#endif
    if (m_Response.status / 100 == 3) {
#ifdef _WIN32
      m_Response.location = m_Response.headers["Location"];
#else
      char* szRedirectUrl = nullptr;
      lStatus = curl_easy_getinfo(m_hSession, CURLINFO_REDIRECT_URL, &szRedirectUrl);
      if (lStatus == CURLE_OK) {
        m_Response.location = szRedirectUrl;
      }
#endif
    }
  }
}

HttpLib::Response* HttpLib::Get()
{
  m_Response.body.clear();
  m_CallBack = &HttpLib::WriteString;
  m_DataBuffer = &m_Response.body;

  HttpPerform();
  return &m_Response;
}

HttpLib::Response* HttpLib::Get(const fs::path& path)
{
  std::ofstream stream(path, std::ios::binary | std::ios::out);
  m_Response.body.clear();
  m_CallBack = &HttpLib::WriteFile;
  if (!stream.is_open())
    return 0;
  m_DataBuffer = &stream;

  HttpPerform();
  stream.close();
  return &m_Response;
}

HttpLib::Response* HttpLib::Get(HttpLib::pCallbackFunction callback, void* userdata) {
  m_Response.body.clear();
  m_CallBack = callback;
  m_DataBuffer = userdata;

  HttpPerform();
  return &m_Response;
}
