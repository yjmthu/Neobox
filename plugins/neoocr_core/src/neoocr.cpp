#include <neoocr.h>
#include <yjson.h>
#include <httplib.h>
#include <pluginmgr.h>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <ranges>
#include <set>
#include <regex>

namespace fs = std::filesystem;

NeoOcr::NeoOcr(YJson& settings, std::function<void()> callback):
  CallBackFunction(callback),
  m_Settings(settings),
  m_TessApi(new tesseract::TessBaseAPI),
  m_TrainedDataDir(fs::path(settings[u8"TessdataDir"].getValueString()).string())
{
  InitLanguagesList();
}

NeoOcr::~NeoOcr()
{
  delete m_TessApi;
}

void NeoOcr::InitLanguagesList()
{
  m_Languages.clear();

  for (const auto& i: m_Settings[u8"Languages"].getArray()) {
    m_Languages += i.getValueString();
    m_Languages.push_back(u8'+');
  }
  
  if (m_Languages.empty()) {
    return;
  }
//  m_Languages.back() = u8'\0';
}

std::u8string NeoOcr::GetText(Pix *pix)
{
  std::u8string result;
  if (m_Languages.empty()) {
    mgr->ShowMsgbox(u8"error", u8"You should set some language first!");
//    m_TessApi->End();
    return result;
  }
  if (m_TessApi->Init(m_TrainedDataDir.c_str(), reinterpret_cast<const char*>(m_Languages.c_str()))) {
    mgr->ShowMsgbox(u8"error", u8"Could not initialize tesseract.");
    return result;
  }
  m_TessApi->SetImage(pix);
  char* szText = m_TessApi->GetUTF8Text();
  result = reinterpret_cast<const char8_t*>(szText);
  m_TessApi->End();
  delete [] szText;
  return result;
}

void NeoOcr::DownloadFile(const std::u8string& url, const fs::path& path)
{
  HttpLib clt(url);
  clt.Get(path);
}

std::u8string NeoOcr::GetLanguageName(const std::u8string& url)
{
  auto iter = std::find(url.crbegin(), url.crend(), u8'/').base();
  return std::u8string(iter, url.end() - 12);
}

void NeoOcr::AddLanguages(const std::vector<std::u8string> &urls)
{
  auto& langsArray = m_Settings[u8"Languages"].getArray();
  auto langsView = langsArray | std::views::transform([](const YJson& item){ return item.getValueString(); });
  std::set<std::u8string> langsSet(langsView.begin(), langsView.end());
  for (const auto& i: urls) {
    if (!i.ends_with(u8".traineddata")) {
      continue;
    }
    auto name = GetLanguageName(i);
    fs::path path = m_Settings[u8"TessdataDir"].getValueString();
    path /= name + u8".traineddata";
    if (langsSet.find(name) == langsSet.end()) {
      langsSet.insert(name);
    }
    if (i.starts_with(u8"http")) {
      DownloadFile(i, path);
    } else if (fs::exists(i)) {
      fs::copy(i, path);
    }
  }
  langsArray.assign(langsView.begin(), langsView.end());
  CallBackFunction();
  InitLanguagesList();
}

void NeoOcr::RmoveLanguages(const std::vector<std::u8string> &names)
{
  auto& jsLangArray = m_Settings[u8"Languages"];
  for (const auto& i: names) {
    jsLangArray.removeByValA(i);
  }
  CallBackFunction();
  InitLanguagesList();
}

void NeoOcr::SetDataDir(const std::u8string &dirname)
{
  auto goodname = dirname | std::views::transform([](const char8_t i) {
    return i == u8'/' ? u8'\\' : i;
  });
  m_Settings[u8"TessdataDir"].getValueString().assign(goodname.begin(), goodname.end());
  CallBackFunction();
}

