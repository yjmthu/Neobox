#include <translate.h>
#include <yjson.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <format>

Translate::Translate():
  m_Data(new YJson (YJson::O {
    { u8"from", u8"auto" },
    { u8"to", u8"zh" },
    { u8"q", YJson::String }
  })),
  m_LanMap({u8"auto", u8"zh", u8"cht", u8"en", u8"jp", u8"fra", u8"ru"})
{
}

Translate::~Translate()
{
  delete m_Data;
}

Translate::Lan Translate::GetCurFromLanguage()
{
  const auto& name = m_Data->find(u8"from")->second.getValueString();
  for (size_t i=0; i != static_cast<size_t>(Lan::MAX); ++i) {
    if (m_LanMap[i] == name) return static_cast<Lan>(i);
  }
  return Lan::AUTO;
}

Translate::Lan Translate::GetCurToLanguage()
{
  const auto& name = m_Data->find(u8"to")->second.getValueString();
  for (size_t i=0; i != static_cast<size_t>(Lan::MAX); ++i) {
    if (m_LanMap[i] == name) return static_cast<Lan>(i);
  }
  return Lan::AUTO;
}

void Translate::SetToLanguage(Lan language)
{
  m_Data->find(u8"to")->second = m_LanMap[static_cast<size_t>(language)];
}

void Translate::SetFromLanguage(Lan language)
{
  m_Data->find(u8"from")->second = m_LanMap[static_cast<size_t>(language)];
}

size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  auto buffer = reinterpret_cast<std::u8string*>(data);
  auto first = reinterpret_cast<const char8_t*>(ptr), last = first + (size*=nmemb);
  buffer->insert(buffer->end(), first, last);
  return size;
}

std::u8string Translate::GetResult(const std::u8string& text)
{
  CURL *pCurl = curl_easy_init();
  curl_slist *pList = nullptr;
  std::u8string response;
  m_Data->find(u8"q")->second = text;

  pList = curl_slist_append(pList, "Content-Type:application/json;charset=utf-8");
  curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pList);
  
#if 1
  curl_easy_setopt(pCurl, CURLOPT_URL, "https://aip.baidubce.com/rpc/2.0/mt/texttrans/v1?access_token=24.9283239c3466a3d58ef8c98a3f65923d.2592000.1665405830.282335-27415445");
#else
  curl_easy_setopt(pCurl, CURLOPT_URL, "https://aip.baidubce.com/rpc/2.0/mt/texttrans-with-dict/v1?access_token=24.9283239c3466a3d58ef8c98a3f65923d.2592000.1665405830.282335-27415445");
#endif
  curl_easy_setopt(pCurl, CURLOPT_HEADER, 0L);
  curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  std::u8string strJsonData = m_Data->toString(false);
  curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
  curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strJsonData.data());
  curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, strJsonData.size());

  auto res = curl_easy_perform(pCurl);
  long lResCode = 0;
  res = curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &lResCode);
  if(( res == CURLE_OK ) && (lResCode == 200 || lResCode == 201)) {
#if 1
    YJson jsData(response.begin(), response.end());
    response.clear();
    if (auto iter = jsData.find(u8"error_msg"); iter != jsData.endO()) {
      response = iter->second.getValueString();
      goto clean;
    }
    const auto& obTransResult = jsData[u8"result"].second[u8"trans_result"].second.getArray();
    for (auto& item: obTransResult) {
      response.append(item[u8"dst"].second.getValueString());
      response.push_back('\n');
    }
#endif
  } else {
    response.clear();
  }
clean:
  curl_slist_free_all(pList); 
  curl_easy_cleanup(pCurl);
  curl_global_cleanup();

  return response;
}

