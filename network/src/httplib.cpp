#include <curl/curl.h>
#include <curl/header.h>
#include <httplib.h>
#include <filesystem>
#include <fstream>
#include <ios>
#include <xstring>

namespace HttpLib {

static size_t WriteFile(void* buffer,
                        size_t size,
                        size_t nmemb,
                        void* userdata) {
  std::ofstream& stream = *reinterpret_cast<std::ofstream*>(userdata);
  stream.write(reinterpret_cast<const char*>(buffer), (size *= nmemb));
  return size;
}

static size_t WriteString(void* buffer,
                          size_t size,
                          size_t nmemb,
                          void* userdata) {
  const char8_t *first = reinterpret_cast<const char8_t*>(buffer),
                *last = first + (size *= nmemb);
  std::u8string& str = *reinterpret_cast<std::u8string*>(userdata);
  str.insert(str.end(), first, last);
  return size;
}

long Get(const char* url, std::u8string& data) {
  CURL* curl = curl_easy_init();
  long status = 0;
  if (!curl)
    return status;
  curl_easy_setopt(curl, CURLOPT_HEADER, false);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteString);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  if (curl_easy_perform(curl) != CURLE_OK)
    return status;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
  curl_easy_cleanup(curl);
  return status;
}

long Get(const char* url, std::filesystem::path path) {
  std::ofstream stream(path, std::ios::binary | std::ios::out);
  if (!stream.is_open())
    return 0;
  CURL* curl = curl_easy_init();
  long status = 0;
  if (!curl)
    return status;
  // struct curl_slist* header_list = NULL;
  // header_list = curl_slist_append(header_list, "User-Agent: Mozilla/5.0
  // (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko");
  // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
  curl_easy_setopt(curl, CURLOPT_HEADER, false);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteFile);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  if (curl_easy_perform(curl) != CURLE_OK)
    return status;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
  curl_easy_cleanup(curl);
  stream.close();
  return status;
}

long Gets(const char* url, std::filesystem::path path) {
  std::u8string data;
  CURL* curl = curl_easy_init();
  long status = 0;
  if (!curl)
    return status;
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_HEADER, false);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteString);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
  if (curl_easy_perform(curl) != CURLE_OK ||
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status) != CURLE_OK)
    return status;
  if (status == 200) {
    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (file.is_open()) {
      file.write(reinterpret_cast<const char*>(data.data()), data.size());
      file.close();
    }
  } else if (status == 301 || status == 302) {
    char* szRedirectUrl = nullptr;
    if (curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &szRedirectUrl) ==
            CURLE_OK &&
        szRedirectUrl) {
      status = Get(szRedirectUrl, path);
    }
  }
  curl_easy_cleanup(curl);

  return status;
}

}  // namespace HttpLib
