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
#include <iostream>
#include <fstream>
#include <format>

using namespace std::literals;
namespace fs = std::filesystem;
std::optional<HttpProxy> HttpLib::m_Proxy = std::nullopt;
// HttpLib::HttpId HttpLib::m_MaxId = 0;
// std::map<HttpLib::HttpId, HttpLib*> HttpLib::m_AsyncPool;
// HttpLib::Mutex HttpLib::m_AsyncMutex;

static HttpLib::HttpId m_MaxId = 0;
static std::map<HttpLib::HttpId, HttpLib*> m_AsyncPool;
static HttpLib::Mutex m_AsyncMutex;

// https://github.com/JGRennison/OpenTTD-patches/blob/dcc52f7696f4ef2601b9fbca1ca78abcd1211734/src/network/core/http_winhttp.cpp#L145
void HttpLib::RequestStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwInternetInformationLength) {
  constexpr auto bufferSize = 8 << 10;
  // sometimes maybe nullptr
  if (!dwContext) return;

  LockerEx locker(m_AsyncMutex);
  auto objectIter = m_AsyncPool.find(dwContext);
  if (objectIter == m_AsyncPool.end())
    return;
  auto& object = *objectIter->second;

  if (object.m_Finished) return;

  switch (dwInternetStatus) {
    case WINHTTP_CALLBACK_STATUS_RESOLVING_NAME:
    case WINHTTP_CALLBACK_STATUS_NAME_RESOLVED:
    case WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER:
    case WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER:
    case WINHTTP_CALLBACK_STATUS_SENDING_REQUEST:
    case WINHTTP_CALLBACK_STATUS_REQUEST_SENT:
    case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE:
    case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED:
    case WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION:
    case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED:
    case WINHTTP_CALLBACK_STATUS_HANDLE_CREATED:
    case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
      /* We don't care about these events, and explicitly ignore them. */
      break;

  case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: 
    locker.unlock();
    WinHttpReceiveResponse(hInternet, nullptr);
    break;
 
  case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: {

    auto bResults = object.ReadStatusCode();
    if (bResults) {
      bResults = object.m_Response.status < 400;
    } else {
      object.EmitFinish(L"HttpLib Error: HttpLib ReadStatusCode Error.");
      break;
    }
    if (bResults) {
      bResults = object.ReadHeaders();
    } else {
      object.EmitFinish(L"HttpLib StatusCode Error.");
      break;
    }
    if (bResults) {
      auto iter = object.m_Response.headers.find(u8"Content-Length");
      if (iter != object.m_Response.headers.end()) {
        std::string number(iter->second.begin(), iter->second.end());
        object.m_ConnectLength = std::stoull(number);
      }
      object.EmitProcess();
      /* Next step: query for any data. */
      locker.unlock();
      WinHttpQueryDataAvailable(hInternet, NULL);
    } else {
      object.EmitFinish(L"HttpLib ReadHeaders Error.");
    }
    break;
  }

  case WINHTTP_CALLBACK_STATUS_REDIRECT:
    /* Make sure we are not in a redirect loop. */
    if (object.m_RedirectDepth++ > 5) {
      object.EmitFinish(L"HTTP request failed: too many redirects");
    }
    break;
  
  case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE: {
    // Retrieve the number of bytes to read
    // Allocate a buffer for the data
    auto buffer = reinterpret_cast<std::u8string*>(object.m_DataBuffer);
    auto size = *reinterpret_cast<DWORD*>(lpvStatusInformation);
    if (!buffer) return;
    buffer->resize(size);
    auto pszOutBuffer = size == 0 ? nullptr : buffer->data();
    // Read the data from the server
    locker.unlock();
    WinHttpReadData(object.m_hRequest, pszOutBuffer, size, nullptr);
    break;
  }

  case WINHTTP_CALLBACK_STATUS_READ_COMPLETE: 
    if (dwInternetInformationLength) {
      object.m_RecieveSize += dwInternetInformationLength;
      object.m_AsyncCallback.m_WriteCallback->operator()(lpvStatusInformation, dwInternetInformationLength);
      object.EmitProcess();

      locker.unlock();
      WinHttpQueryDataAvailable(object.m_hRequest, nullptr);
    } else{
      object.EmitFinish();
    }
    break;

  case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
  case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: {
    auto const* pAsyncResult = (WINHTTP_ASYNC_RESULT*)lpvStatusInformation;
    DWORD dwError = pAsyncResult->dwError; // The error code
    DWORD dwResult = pAsyncResult->dwResult; // The ID of the called function
    object.EmitFinish(std::format(L"Winhttp status error. Error code: {}, error id: {}.", dwError, dwResult));
    break;
  }
  }
}

HttpUrl::HttpUrl(std::u8string_view url) {
  SetUrl(url);
}

HttpUrl::HttpUrl(StringView host,
  StringView path,
  Params params,
  StringView scheme,
  uint16_t port
)
  : scheme(scheme)
  , host(host)
  , path(path)
  , port(port)
  , parameters(std::move(params))
{
  auto iter = host.cbegin();
  ParseHost(iter, host.end());
  if (iter != host.end()) {
    ParsePort(iter, host.end());
  }
  iter = path.cbegin();
  ParsePath(iter, path.cend());
}

HttpUrl::HttpUrl(HttpUrl&& url) noexcept
  : scheme(std::move(url.scheme))
  , host(std::move(url.host))
  , path(std::move(url.path))
  , port(url.port)
  , parameters(std::move(url.parameters))
{
}

HttpUrl::HttpUrl(const HttpUrl& url)
  : scheme(url.scheme)
  , host(url.host)
  , path(url.path)
  , port(url.port)
  , parameters(url.parameters)
{
}

HttpUrl::HttpUrl(StringView url, Params params)
  : parameters(std::move(params))
{
  SetUrl(url);
}

void HttpUrl::ParseScheme(Iterator& first, Iterator last)
{
  scheme = u8"https";
  if (last - first > 7 && std::equal(first, first + 4, u8"http")) {
    first += 4;
    if (*first == u8's') {
      ++first;
    } else {
      scheme.pop_back();
    }
    if (std::equal(first, first + 3, u8"://")) {
      first += 3;
    } else {
      throw std::logic_error("HttpUrl Error: Url scheme doesn't end with '://'.");
    }
  } else {
    throw std::logic_error("HttpUrl Error: Url scheme doesn't start with 'http(s)://'.");
  }
}

void HttpUrl::ParseHost(Iterator& first, Iterator last)
{
  auto iter = std::find(first, last, u8'/');
  if (first == iter || *first == u8':') {
    throw std::logic_error("HttpUrl Error: Url host doesn't exist!");
  }
  host.assign(first, iter);

  auto i = host.rfind(u8':');
  if (i == host.npos) {
    first = iter;
    return;
  }
  first += i;
  host.erase(i, host.size() - i);
}

void HttpUrl::ParsePort(Iterator& first, Iterator last)
{
  if (scheme.back() == u8's') {
    port = 443;
  } else {
    port = 80;
  }
  if (first == last) return;

  switch (*first) {
    case ':':{
      port = 0;
      while (++first != last && u8'0' <= *first && *first <= u8'9') {
        port *= 10;
        port += *first - u8'0';
      }
      break;
    }
    case '/':
      return;
    default:
      throw std::logic_error("HttpUrl Error: Url is invalid!");
  }
}

void HttpUrl::ParsePath(Iterator& first, Iterator last)
{
  if (first == last) {
    path = u8"/";
    return;
  }
  if (*first != u8'/') {
    throw std::logic_error("HttpUrl Error: Url path doesn't start with '/'.");
  }

  std::u8string_view str(first, last);
  auto i = str.find(u8'?');
  if (i == str.npos) {
    path = str;
    first = last;
    return;
  }
  path = str.substr(0, ++i);
  first += i;
}

void HttpUrl::ParseParams(Iterator& first, Iterator last)
{
  while (first != last) {
    auto e = std::find(first, last, u8'&');
    auto i = std::find(first, e, u8'=');
    if (i != e) {
      String key;
      UrlDecode(StringView(first, i), key);
      UrlDecode(StringView(i + 1, e), parameters[key]);
    }
    first = e != last ? ++e: e;
  };
}

void HttpUrl::UrlEncode(StringView text, String& out)
{
  for (auto i: text) {
    if (std::isalnum(i) || u8"-_.~"s.find(i) != std::u8string_view::npos) {
      out.push_back(i);
    } else {
      out.push_back('%');
      out.push_back(ToHex(i >> 4));
      out.push_back(ToHex(i & 0xF));
    }
  }
}

void HttpUrl::UrlDecode(StringView text, String& out)
{
  for (auto i = text.cbegin(); i != text.cend(); ++i) {
    switch (*i) {
      case u8'%':{
        if (text.cend() - i < 2) {
          throw std::logic_error("HttpUrl Error: Url decode error.");
        }
        auto const c = FromHex(*++i) << 4;
        out.push_back(c | FromHex(*++i));
        break;
      }
      case u8'+':
        out.push_back(u8' ');
        break;
      default:
        out.push_back(*i);
        break;
    }
  }
}

HttpUrl::String HttpUrl::GetUrl(bool showPort) const
{
  auto url = scheme + u8"://" + host;
  if (showPort) {
    url.push_back(u8':');
    auto str = std::to_string(port);
    url.append(str.begin(), str.end());
  }
  url += GetObjectString();

  return url;
}

void HttpUrl::SetUrl(StringView url)
{
  auto iter = url.cbegin();

  ParseScheme(iter, url.cend());
  ParseHost(iter, url.cend());
  ParsePort(iter, url.cend());
  ParsePath(iter, url.cend());
  ParseParams(iter, url.cend());
}

HttpUrl::String HttpUrl::GetObjectString() const
{
  auto object = path;
  for (auto& [key, value]: parameters) {
    UrlEncode(key, object);
    object.push_back(u8'=');
    UrlEncode(value, object);
    object.push_back(u8'&');
  }

  if (!parameters.empty()) {
    object.pop_back();
  }

  return object;
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
  HttpLib clt("https://www.baidu.com"s);
  auto res = clt.Get();
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
  if (m_AsyncSet) {
    ExitAsync();
    m_AsyncMutex.lock();
    m_AsyncPool.erase(m_AsyncId);
    m_AsyncMutex.unlock();
  } else {
    m_Finished = true;
    HttpUninitialize();
  }
}

void HttpLib::SetAsyncCallback()
{
#ifdef _WIN32
  // auto ctx = this;
  // auto bResult = WinHttpSetOption(
  //   m_hSession,
  //   WINHTTP_OPTION_CONTEXT_VALUE,
  //   &ctx,
  //   sizeof(void*)
  // );
  // if (bResult) {
    WINHTTP_STATUS_CALLBACK hCallback = WinHttpSetStatusCallback(
      m_hSession,
      RequestStatusCallback,
      WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
      NULL
    );
  // } else {
  //   throw std::logic_error(std::format("Httplib SetAsyncCallback failed. Code: {}.", GetLastError()));
  // }
#endif
}

void HttpLib::IntoPool()
{
  if (m_AsyncSet) {
    m_AsyncMutex.lock();
    m_AsyncId = ++m_MaxId;
    m_AsyncPool[m_AsyncId] = this;
    m_AsyncMutex.unlock();
  }
}

void HttpLib::HttpPrepare()
{
#ifdef _WIN32
  if (!WinHttpCheckPlatform()) {
    throw std::logic_error("This platform is NOT supported by WinHTTP.");
  }
#endif
}

void HttpLib::ResetData() {
  m_PostData.data = nullptr;
  m_PostData.size = 0;
  m_RecieveSize = 0;
  m_ConnectLength = 0;
  m_Finished = false;
}

void HttpLib::HttpInitialize()
{
  ResetData();
#ifdef _WIN32
  m_hSession = WinHttpOpen(L"WinHTTP in Neobox/1.0", 
    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
    WINHTTP_NO_PROXY_NAME, 
    WINHTTP_NO_PROXY_BYPASS,
    m_AsyncSet ? WINHTTP_FLAG_ASYNC: 0);
  if (!m_hSession) {
    std::wcerr << L"HttpLib Error: " << GetLastError() << L" in WinHttpOpen.\n";
    return;
  }

  if (m_AsyncSet) {
    SetAsyncCallback();
  }
  auto url = Utf82WideString(m_Url.host);

  SetTimeOut(m_TimeOut);
  m_hConnect = WinHttpConnect(m_hSession, url.c_str(), m_Url.port, 0);
  if (m_hConnect) {
    SetProxyBefore();
  } else {
    std::wcerr << L"HttpLib Error: " << GetLastError() << L" in WinHttpConnect.\n";
  }

#else
  SetProxyBefore();
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
  if (m_TimeOut > 0) {
    curl_easy_setopt(m_hSession, CURLOPT_TIMEOUT, m_TimeOut);
  }
#endif
}


void HttpLib::HttpUninitialize() {
#ifdef _WIN32
  if(m_hRequest) {
    WinHttpCloseHandle(m_hRequest);
    m_hRequest = nullptr;
  }
  if (m_hConnect) {
    WinHttpCloseHandle(m_hConnect);
    m_hConnect = nullptr;
  }
  if (m_hSession) {
    WinHttpCloseHandle(m_hSession);
    m_hSession = nullptr;
  }
#else
  if (m_hSession)
    curl_easy_cleanup(m_hSession);
#endif
}

void HttpLib::SetProxyBefore()
{
  if (!m_Proxy) return;

  auto const type = static_cast<int>(m_Proxy->GetType());
  if (type == 3) return;

#ifdef _WIN32
  if (type == 0) {
    if (!m_Proxy->IsSystemProxy()) return;
    m_Proxy->UpdateSystemProxy();
  }
  auto proxyString = Utf82WideString(m_Proxy->GetProxy());
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
    if (m_Proxy->IsUserEmpty()) return;
    m_ProxySet = true;
  }
#else
  // https://curl.se/libcurl/c/CURLOPT_PROXY.html
  // https://curl.se/libcurl/c/CURLOPT_PROXYAUTH.html
  auto pwd = m_Proxy.username + u8":" + m_Proxy.password;
  curl_easy_setopt(m_hSession, CURLOPT_PROXY, m_Proxy.proxy.c_str());
  /* allow whatever auth the proxy speaks */
  curl_easy_setopt(m_hSession, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
  /* set the proxy credentials */
  if (pwd.length() > 3)
    curl_easy_setopt(m_hSession, CURLOPT_PROXYUSERPWD, pwd.c_str());
#endif
}

bool HttpLib::SetProxyAfter()
{
  bool bResult = true;
  if (!m_ProxySet || !m_Proxy) return true;

#ifdef _WIN32
  auto username = Utf82WideString(m_Proxy->GetUsername());
  auto password = Utf82WideString(m_Proxy->GetPassword());

  bResult = WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_USERNAME, username.data(), username.size() * sizeof(wchar_t));
  if (bResult) {
    bResult = WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_PASSWORD, password.data(), password.size() * sizeof(wchar_t));
  } else {
    std::wcerr << L"Winhttp SetProxyAfter Failed!" << std::endl;
  }
#endif
  return bResult;
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
bool HttpLib::SendRequest()
{
  if (m_PostData.data) {
#ifdef _WIN32
    return WinHttpSendRequest(m_hRequest,
      WINHTTP_NO_ADDITIONAL_HEADERS, 0,
      m_PostData.data, m_PostData.size,
      m_PostData.size,
      static_cast<DWORD_PTR>(m_AsyncId)
    );
#else
    curl_easy_setopt(m_hSession, CURLOPT_POST, 1L);
    curl_easy_setopt(m_hSession, CURLOPT_POSTFIELDS, m_PostData.data);
    curl_easy_setopt(m_hSession, CURLOPT_POSTFIELDSIZE, m_PostData.size);
#endif
  } else {
#ifdef _WIN32
    return WinHttpSendRequest(m_hRequest,
      WINHTTP_NO_ADDITIONAL_HEADERS, 0,
      WINHTTP_NO_REQUEST_DATA, 0, 0,
      static_cast<DWORD_PTR>(m_AsyncId)
    );
#endif
  }
  return true;
}

bool HttpLib::RecvResponse() {
  bool bResults = false;
#ifdef _WIN32
  bResults = WinHttpReceiveResponse(m_hRequest, NULL);
#else
  auto lStatus = curl_easy_perform(m_hSession);
  bResults = lStatus == CURLE_OK;
#endif
  return bResults;

}

bool HttpLib::SendHeaders()
{
  bool bResults = false;
#ifdef _WIN32
  auto path = Utf82WideString(m_Url.GetObjectString());
  m_hRequest = WinHttpOpenRequest(
    m_hConnect,
    m_PostData.data ? L"POST": L"GET",
    path.c_str(),
    nullptr,
    WINHTTP_NO_REFERER,
    WINHTTP_DEFAULT_ACCEPT_TYPES,
    m_Url.IsHttps() ? WINHTTP_FLAG_SECURE : 0
  );
  if (m_hRequest) {
    bResults = SetProxyAfter();
  } else {
    std::wcerr << L"WinHttp openRequest failed!" << std::endl;
  }
  if (bResults) {
    for (auto& [i, j]: m_Headers) {
      auto header = Utf82WideString(i + u8": " + j);
      bResults = WinHttpAddRequestHeaders(m_hRequest, header.data(), header.size(), WINHTTP_ADDREQ_FLAG_ADD);
      if (!bResults) {
        std::wcerr << L"Winhttp add headers failed!" << std::endl;
        break;
      }
    }
  } else {
    std::wcerr << L"Winhttp setProxyAfter failed!" << std::endl;
  }
#else
  curl_easy_setopt(m_hSession, CURLOPT_WRITEFUNCTION, m_CallBack);
  curl_easy_setopt(m_hSession, CURLOPT_WRITEDATA, m_DataBuffer);
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
  return bResults;
}


bool HttpLib::ReadStatusCode() 
{
  bool bResults = false;
#ifdef _WIN32
  DWORD dwSize = sizeof(m_Response.status);
  bResults = WinHttpQueryHeaders(m_hRequest, 
      WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
      WINHTTP_HEADER_NAME_BY_INDEX, &m_Response.status, &dwSize, WINHTTP_NO_HEADER_INDEX);

  if (bResults) {
    if(m_Response.status == 304) 
      std::wcerr << L"Document has not been updated.\n";
  }
#else
  if (bResults)
    lStatus = curl_easy_getinfo(m_hSession, CURLINFO_RESPONSE_CODE, &m_Response.status);
  else
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(lStatus) << std::endl;
  bResults = lStatus == CURLE_OK;
#endif
  return bResults;
}

bool HttpLib::ReadHeaders()
{
  bool bResults = false;
  m_Response.headers.clear();

#ifdef _WIN32
  // Query the status code of the response
  DWORD dwSize = 0;

  WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, NULL, NULL, &dwSize, NULL);
  if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
    auto const lpOutBuffer = new WCHAR[dwSize/sizeof(WCHAR)];

    // Now, use WinHttpQueryHeaders to retrieve the header.
    bResults = WinHttpQueryHeaders(m_hRequest,
                                WINHTTP_QUERY_RAW_HEADERS_CRLF,
                                WINHTTP_HEADER_NAME_BY_INDEX,
                                lpOutBuffer, &dwSize,
                                WINHTTP_NO_HEADER_INDEX);

    if (bResults) { // regex expr: '/^([^:]+):([^\n]+)/'
      // std::istringstream strstream(Wide2AnsiString(lpOutBuffer));
      auto const outBuffer = Wide2Utf8String(lpOutBuffer);
      auto cursor = outBuffer.find(u8"\r\n");
      if (cursor != outBuffer.npos) {
        auto const pos = outBuffer.find(u8' ');
        m_Response.version = outBuffer.substr(0, pos);
        cursor += 2;
      }
      for (; cursor != outBuffer.npos; cursor += 2) {
        auto mid = outBuffer.find(u8':', cursor);
        cursor = outBuffer.find(u8"\r\n", mid);
        if (mid == outBuffer.npos || cursor == outBuffer.npos) {
          break;
        }
        auto key = outBuffer.substr(0, mid);
        mid = outBuffer.find_first_not_of(u8' ', ++mid);
        m_Response.headers[key] = outBuffer.substr(mid, cursor - mid);
      }

      if (m_Response.status / 100 == 3) {
#ifdef _WIN32
        m_Response.location = m_Response.headers[u8"Location"];
#else
      char* szRedirectUrl = nullptr;
      lStatus = curl_easy_getinfo(m_hSession, CURLINFO_REDIRECT_URL, &szRedirectUrl);
      if (lStatus == CURLE_OK) {
        m_Response.location = szRedirectUrl;
      }
#endif
      }
    }

    delete[] lpOutBuffer;
  } else {
    std::wcerr << L"WinHttpQueryHeaders BufferSize Failed." << std::endl;
  }
#else
    char *ct = nullptr;
    lStatus = curl_easy_getinfo(m_hSession, CURLINFO_CONTENT_TYPE, &ct);
    if(!lStatus && ct) {
      m_Response.headers["Content-Type"] = ct;
    }
#endif

  return bResults;
}

bool HttpLib::ReadBody()
{
  bool bResults = false;
  DWORD dwSize = 0;

  std::string strBuffer;
  for (DWORD dwDownloaded = 0; !m_Finished; ) {
    bResults = WinHttpQueryDataAvailable(m_hRequest, &dwSize);
    if (!bResults) {
      std::wcerr << L"WinHttpQueryDataAvailable failed: " << GetLastError() << std::endl;
      break;
    }           
    if (dwSize == 0) break;
    strBuffer.resize(dwSize);
    bResults = WinHttpReadData(m_hRequest, strBuffer.data(), dwSize, &dwDownloaded);
    if (!bResults) {
      std::wcerr << L"WinHttpQueryDataAvailable failed: " << GetLastError() << std::endl;
    }
    if (!dwDownloaded) break;
    m_Callback(strBuffer.data(), sizeof(char), dwDownloaded, m_DataBuffer);
  };

  return bResults;
}

void HttpLib::HttpPerform()
{
  bool bResults = SendHeaders();

  if (bResults) {
    bResults = SendRequest();
  } else {
    std::wcerr << L"WinHttpSendHeasers failed: " << GetLastError() << std::endl;
  }
  if (m_AsyncSet) {
    if (!bResults) {
      std::thread(&HttpLib::ExitAsync, this).detach();
    }
    return;
  }

  if (bResults) {
    bResults = RecvResponse();
  } else {
    std::wcerr << L"WinHttpSendRequest failed: " << GetLastError() << std::endl;
  }
  if (bResults) {
    bResults = ReadStatusCode();
  } else {
    std::wcerr << L"WinHttpReceiveResponse failed: " << GetLastError() << std::endl;
  }

  if (bResults) {
    bResults = m_Response.status == 200;
  } else {
    std::wcerr << L"WinHttp read status code failed: " << GetLastError() << std::endl;
  }
  
  if (bResults) {
    bResults = ReadHeaders();
  } else {
    std::wcerr << L"Winhttp status code is " << m_Response.status << L".\n";
  }

  if(bResults) {
    bResults = ReadBody();
  } else {
    std::wcerr << L"WinHttpQueryHeaders BufferSize Failed." << std::endl;
  }
}

void HttpLib::EmitProcess()
{
  const auto& callback = m_AsyncCallback.m_ProcessCallback;
  if (callback) {
    (*callback)(m_RecieveSize, m_ConnectLength);
  }
}

void HttpLib::EmitFinish(std::wstring message)
{
  m_Finished = true;
  if (m_hSession) {
    WinHttpSetStatusCallback(
      m_hSession,
      NULL,
      WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
      NULL
    );
  }
  auto& callback = m_AsyncCallback.m_FinishCallback;
  if (callback) {
    /* To prevent infinite recursion at destructor time,
      the callback function is emptied after one execution.
      */
    auto cb = std::move(*callback);
    callback = std::nullopt;
    cb(message, &m_Response);
  }
  // Locker locker(m_AsyncMutex);
   delete reinterpret_cast<std::u8string*>(m_DataBuffer);
   m_DataBuffer = nullptr;
}

HttpLib::Response* HttpLib::Get()
{
  m_Response.body.clear();
  m_Callback = &HttpLib::WriteString;
  m_DataBuffer = &m_Response.body;

  HttpPerform();
  return &m_Response;
}

HttpLib::Response* HttpLib::Get(const fs::path& path)
{
  fs::path tempPath = GetTempFileName();
  if (tempPath.empty()) return nullptr;

  std::ofstream stream(tempPath, std::ios::binary | std::ios::out);
  m_Response.body.clear();
  m_Callback = &HttpLib::WriteFile;
  if (!stream.is_open())
    return 0;
  m_DataBuffer = &stream;

  HttpPerform();
  stream.close();

  if (!fs::file_size(tempPath)) return nullptr;

  fs::rename(tempPath, path);
  return &m_Response;
}

HttpLib::Response* HttpLib::Get(CallbackFunction* callback, void* userdata) {
  m_Response.body.clear();
  m_Callback = callback;
  m_DataBuffer = userdata;

  HttpPerform();
  return &m_Response;
}

void HttpLib::SetTimeOut(int TimeOut) {
  // Use WinHttpSetTimeouts to set a new time-out values.
  m_TimeOut = TimeOut;
  const auto timeout = m_TimeOut * 1000;
  if (!WinHttpSetTimeouts(m_hSession, timeout, timeout, timeout, timeout)) {
    std::wcerr << L"HttpLib Error: " << GetLastError() << L" in WinHttpSetTimeouts.\n";
  }
}

void HttpLib::GetAsync(Callback callback)
{
  if (!m_AsyncSet) {
    throw std::logic_error("HttpLib Error: HttpAync wasn't set!");
  }

  m_Finished = false;

  m_Response.body.clear();
  m_AsyncCallback = std::move(callback);
  m_DataBuffer = new std::u8string;
  if (!m_AsyncCallback.m_WriteCallback) {
    m_AsyncCallback.m_WriteCallback = [this](auto data, auto size){
      m_Response.body.append(reinterpret_cast<const char8_t*>(data), size);
    };
  }

  if (!m_hConnect || !m_hSession) {
    std::thread(&HttpLib::ExitAsync, this).detach();
    return;
  }

  HttpPerform();
}

void HttpLib::ExitAsync() {
  m_AsyncMutex.lock();
  m_Response.status = -1;
  EmitFinish(L"Httplib Error: User terminate.");
  m_AsyncMutex.unlock();
  HttpUninitialize();
}
