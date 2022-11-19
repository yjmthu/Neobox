#include <curl/curl.h>
#include <curl/header.h>

#ifdef _WIN32
#include <Shlobj.h>
#include <wininet.h>
#endif  // _WIN32

#include <httplib.h>
#include <filesystem>
#include <fstream>
#include <format>

bool HttpLib::IsOnline() {
#ifdef _WIN32
  DWORD flags;
  return InternetGetConnectedState(&flags, 0);
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
  curl_easy_cleanup(m_Curl);
}

void HttpLib::CurlInit()
{
  if (m_Curl)
    curl_easy_cleanup(m_Curl);
  m_Curl = curl_easy_init();
  curl_easy_setopt(m_Curl, CURLOPT_HEADER, false);
  curl_easy_setopt(m_Curl, CURLOPT_URL, m_Url.data());
  curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(m_Curl, CURLOPT_SSL_VERIFYHOST, false);
  curl_easy_setopt(m_Curl, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(m_Curl, CURLOPT_NOSIGNAL, 1);
}

void HttpLib::SetRedirect(long redirect)
{
  curl_easy_setopt(m_Curl, CURLOPT_FOLLOWLOCATION, redirect);
}

void HttpLib::CurlPerform()
{
  CURLcode lStatus;
  if (!m_Headers.empty()) {
    struct curl_slist *headers = nullptr;
    std::string buffer;
    for (const auto& [key, value]: m_Headers) {
      buffer = std::format("{}: {}", key, value);
      buffer.push_back('\0');
      headers = curl_slist_append(headers, buffer.data());
    }
    curl_easy_setopt(m_Curl, CURLOPT_HTTPHEADER, headers);
  }
  lStatus = curl_easy_perform(m_Curl);
  if (lStatus != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(lStatus));
  } else {
    curl_easy_getinfo(m_Curl, CURLINFO_RESPONSE_CODE, &m_Response.status);
    if((lStatus == CURLE_OK) && ((m_Response.status / 100) != 3)) {
      char *ct = nullptr;
      m_Response.headers.clear();
      lStatus = curl_easy_getinfo(m_Curl, CURLINFO_CONTENT_TYPE, &ct);
      if(!lStatus && ct) {
        m_Response.headers["Content-Type"] = ct;
      }
    } else {
      char* szRedirectUrl = nullptr;
      lStatus = curl_easy_getinfo(m_Curl, CURLINFO_REDIRECT_URL, &szRedirectUrl);
      if (lStatus == CURLE_OK) {
        m_Response.location = szRedirectUrl;
      }
    }
  }
}

HttpLib::Response* HttpLib::Get()
{
  m_Response.body.clear();
  
  curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, &HttpLib::WriteString);
  curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &m_Response.body);

  CurlPerform();
  return &m_Response;
}

HttpLib::Response* HttpLib::Get(const std::filesystem::path& path)
{
  std::ofstream stream(path, std::ios::binary | std::ios::out);
  if (!stream.is_open())
    return 0;
  curl_easy_setopt(m_Curl, CURLOPT_WRITEFUNCTION, &WriteFile);
  curl_easy_setopt(m_Curl, CURLOPT_WRITEDATA, &stream);

  CurlPerform();
  stream.close();
  return &m_Response;
}

