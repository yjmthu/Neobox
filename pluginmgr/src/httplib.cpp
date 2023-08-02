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

// https://github.com/JGRennison/OpenTTD-patches/blob/dcc52f7696f4ef2601b9fbca1ca78abcd1211734/src/network/core/http_winhttp.cpp#L145
void HttpLib::RequestStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwInternetInformationLength) {
  constexpr auto bufferSize = 8 << 10;
  if (!dwContext) {
    throw std::logic_error("HttpLib Error: No context in callback!");
  }
  auto object = reinterpret_cast<HttpLib*>(dwContext);
  if (object->m_Finished)
    return;
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
    WinHttpReceiveResponse(object->m_hRequest, nullptr);
    break;
 
  case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: {
    auto bResults = object->ReadStatusCode();
    if (bResults) {
      bResults = object->m_Response.status < 400;
    } else {
      // locker.unlock();
      object->EmitFinish(L"HttpLib Error: HttpLib ReadStatusCode Error.");
      break;
    }
    if (bResults) {
      bResults = object->ReadHeaders();
    } else {
      // locker.unlock();
      object->EmitFinish(L"HttpLib StatusCode Error.");
      break;
    }
    if (bResults) {
      auto iter = object->m_Response.headers.find("Content-Length");
      if (iter != object->m_Response.headers.end()) {
        object->m_ConnectLength = std::stoull(iter->second);
      }
      object->EmitProcess();
      /* Next step: query for any data. */
      WinHttpQueryDataAvailable(object->m_hRequest, NULL);
    } else {
      // locker.unlock();
      object->EmitFinish(L"HttpLib ReadHeaders Error.");
    }
    break;
  }

  case WINHTTP_CALLBACK_STATUS_REDIRECT:
    /* Make sure we are not in a redirect loop. */
    if (object->m_RedirectDepth++ > 5) {
      // locker.unlock();
      object->EmitFinish(L"HTTP request failed: too many redirects");
    }
    break;

  case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE: {
    // Retrieve the number of bytes to read
    // Allocate a buffer for the data
    auto buffer = reinterpret_cast<std::u8string*>(object->m_DataBuffer);
    buffer->resize(*(DWORD *)lpvStatusInformation);
    auto pszOutBuffer = buffer->empty() ? nullptr : buffer->data();
    // Read the data from the server
    WinHttpReadData(object->m_hRequest, pszOutBuffer, buffer->size(), nullptr);
    break;
  }

  case WINHTTP_CALLBACK_STATUS_READ_COMPLETE: {
    if (!object->m_Finished) {
      object->m_RecieveSize += dwInternetInformationLength;
      object->m_AsyncCallback.m_WriteCallback->operator()(lpvStatusInformation, dwInternetInformationLength);
      object->EmitProcess();
    }
    if (object->m_Finished) {
      return;
    } else if (dwInternetInformationLength == 0) {
      // locker.unlock();
      object->EmitFinish();
    } else {
      WinHttpQueryDataAvailable(object->m_hRequest, nullptr);
    }
    break;
  }

  case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
  case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: {
    if (object->m_Finished) return;
    auto const* pAsyncResult = (WINHTTP_ASYNC_RESULT*)lpvStatusInformation;
    DWORD dwError = pAsyncResult->dwError; // The error code
    DWORD dwResult = pAsyncResult->dwResult; // The ID of the called function
    // locker.unlock();
    object->EmitFinish(std::format(L"Winhttp status error. Error code: {}, error id: {}.", dwError, dwResult));
    break;
  }
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
  } else {
    m_Finished = true;
    HttpUninitialize();
  }
}

void HttpLib::SetAsyncCallback()
{
#ifdef _WIN32
  auto ctx = this;
  auto bResult = WinHttpSetOption(
    m_hSession,
    WINHTTP_OPTION_CONTEXT_VALUE,
    &ctx,
    sizeof(void*)
  );
  if (bResult) {
    WINHTTP_STATUS_CALLBACK hCallback = WinHttpSetStatusCallback(
      m_hSession,
      RequestStatusCallback,
      WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
      NULL
    );
  } else {
    throw std::logic_error(std::format("Httplib SetAsyncCallback failed. Code: {}.", GetLastError()));
  }
#endif
}

std::u8string HttpLib::GetDomain()
{
  std::u8string result;
#ifdef __linux__
  std::u8string::const_iterator iter;
#endif
  if (m_Url.starts_with(u8"https://")) {
#ifdef _WIN32
    result = m_Url.substr(8, m_Url.find('/', 8) - 8);
#else
    iter = m_Url.cbegin() + 8;
#endif
  } else if (m_Url.starts_with(u8"http://")) {
#ifdef _WIN32
    result = m_Url.substr(7, m_Url.find('/', 7) - 7);
#else
    iter = m_Url.cbegin() + 7;
#endif
  } else {
    throw std::logic_error("Url should begin with 'http://' or 'https://'!");
  }
#ifndef _WIN32
  result.assign(iter, std::find(iter, m_Url.cend(), '/'));
#endif
  // result.push_back(L'\0');
  return result;
}

std::u8string HttpLib::GetPath()
{
  size_t pos;
  if (m_Url.starts_with(u8"https://")) {
    pos = m_Url.find('/', 8);
  } else if (m_Url.starts_with(u8"http://")) {
    pos = m_Url.find('/', 7);
  } else {
    throw std::logic_error("Url should begin with 'http://' or 'http://'!");
  }
  if (pos == std::string::npos) {
    return u8"/"s;
  }
#ifdef _WIN32
  auto result = m_Url.substr(pos);
#else
  String result(m_Url.begin() + pos, m_Url.end());
#endif
  return result;
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
    return;
  }

  if (m_AsyncSet) {
    SetAsyncCallback();
  }
  auto url = Utf82WideString(GetDomain());

  // Use WinHttpSetTimeouts to set a new time-out values.
  const auto timeout = m_TimeOut * 1000;
  if (!WinHttpSetTimeouts(m_hSession, timeout, timeout, timeout, timeout)) {
    std::wcout << L"Error " << GetLastError() << L" in WinHttpSetTimeouts.\n";
  }
  m_hConnect = WinHttpConnect(m_hSession, url.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);

  SetProxyBefore();

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
      WINHTTP_NO_ADDITIONAL_HEADERS, 0, m_PostData.data, m_PostData.size, m_PostData.size, 0);
#else
    curl_easy_setopt(m_hSession, CURLOPT_POST, 1L);
    curl_easy_setopt(m_hSession, CURLOPT_POSTFIELDS, m_PostData.data);
    curl_easy_setopt(m_hSession, CURLOPT_POSTFIELDSIZE, m_PostData.size);
#endif
  } else {
#ifdef _WIN32
    return WinHttpSendRequest(m_hRequest,
      WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
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
  auto path = Utf82WideString(GetPath());
  m_hRequest = WinHttpOpenRequest (m_hConnect,
    m_PostData.data ? L"POST": L"GET",
    path.c_str(), L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
  if (m_hRequest) {
    bResults = SetProxyAfter();
  } else {
    std::wcerr << L"WinHttp openRequest failed!" << std::endl;
  }
  if (bResults) {
    for (auto& [i, j]: m_Headers) {
      auto header = Ansi2WideString(std::format("{}: {}", i, j));
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
      std::wcout << L"Document has not been updated.\n";
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
      std::wcout << L"WinHttpQueryDataAvailable failed: " << GetLastError() << std::endl;
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
  if (m_AsyncSet) return;

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
  auto& callback = m_AsyncCallback.m_FinishCallback;
  if (callback) {
    /* To prevent infinite recursion at destructor time,
      the callback function is emptied after one execution.
      */
    auto cb = std::move(*callback);
    callback = std::nullopt;
    cb(message, &m_Response);
  }
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
      m_Response.body.append(reinterpret_cast<const char*>(data), size);
    };
  }
  HttpPerform();
}

void HttpLib::ExitAsync() {
  m_Response.status = -1;
  EmitFinish(L"Httplib Error: User terminate.");
  HttpInitialize();
}