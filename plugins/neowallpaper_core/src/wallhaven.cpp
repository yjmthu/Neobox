#include <wallhaven.h>
#include <httplib.h>
#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <regex>
#include <utility>
#include <numeric>
#include <functional>
#include <array>
#include <filesystem>


using namespace std::literals;

Wallhaven::Wallhaven(YJson& setting):
  WallBase(InitSetting(setting)),
  m_Data(nullptr)
{
}

Wallhaven::~Wallhaven()
{
  delete m_Data;
}


bool Wallhaven::IsPngFile(std::u8string& str) {
  // if (!Wallpaper::IsOnline()) return false;
  HttpLib clt(u8"https://wallhaven.cc/api/v1/w/"s + str);
  auto res = clt.Get();
  if (res->status != 200)
    return false;
  YJson js(res->body.begin(), res->body.end());
  str = js[u8"data"][u8"path"].getValueString().substr(31);
  return true;
}

YJson& Wallhaven::InitSetting(YJson& setting)
{
  if (setting.isObject()) {
    return setting;
  }
  setting = YJson::O {
    {u8"WallhavenCurrent", u8"最热壁纸"},
    {u8"WallhavenApi", YJson::O {}},
    {u8"ApiKey", YJson::Null},
    {u8"PageSize", 5},
  };
  std::initializer_list<std::tuple<std::u8string, bool, std::u8string, std::u8string>>
    paramLIst = {
      {u8"最热壁纸"s, true, u8"categories"s, u8"111"s},
      {u8"风景壁纸"s, true, u8"q"s, u8"nature"s},
      {u8"动漫壁纸"s, true, u8"categories"s, u8"010"s},
      {u8"随机壁纸"s, false, u8"sorting"s, u8"random"s},
      {u8"极简壁纸"s, false, u8"q"s, u8"minimalism"s},
      {u8"鬼刀壁纸"s, false, u8"q"s, u8"ghostblade"s}
  };
  auto& m_ApiObject = setting[u8"WallhavenApi"];
  const auto initDir = GetStantardDir(u8"壁纸天堂");

  for (const auto& [i, j, k, l] : paramLIst) {
    auto& item = m_ApiObject.append(YJson::Object, i)->second;
    item[u8"Parameter"] = j ? YJson::O {
      {u8"sorting", u8"toplist"}, {k, l}}: YJson::O {{k, l},};
    item.append(initDir, u8"Directory");
    item.append(1, u8"StartPage");
  }
  SaveSetting();
  return setting;
}

std::u8string Wallhaven::GetApiPathUrl() const {
  const auto& apiInfo = GetCurInfo();
  std::u8string result;
  auto& param = apiInfo[u8"Parameter"];
  if (param.isObject()) {
    result = param.urlEncode(u8"https://wallhaven.cc/api/v1/search?"sv);
  } else if (param.isString()) {
    result = param.getValueString();
  } else {
    throw std::runtime_error("Cant find Wallhaven Parameter or Path!"s);
  }
  auto& ApiKey = m_Setting[u8"ApiKey"];
  if (ApiKey.isString()) {
    result.append(u8"&apikey="s + ApiKey.getValueString());
  }
  return result;
}

YJson& Wallhaven::GetCurInfo()
{
  return m_Setting[u8"WallhavenApi"][m_Setting[u8"WallhavenCurrent"].getValueString()];
}

bool Wallhaven::CheckData(ImageInfoEx ptr)
{
  auto const apiUrl = GetApiPathUrl();
  std::u8string curUrl;
  if (m_Data) {
    curUrl = m_Data->find(u8"Api")->second.getValueString();
  } else if (fs::exists(m_DataPath)) {
    m_Data = new YJson(m_DataPath, YJson::UTF8);
    curUrl = m_Data->find(u8"Api")->second.getValueString();
  }

  if (curUrl == apiUrl) {
    if (m_Data->find(u8"Unused")->second.emptyA() &&
      m_Data->find(u8"Used")->second.emptyA()) {
      ptr->ErrorMsg = u8"No data has been downloaded.";
      ptr->ErrorCode = ImageInfo::DataErr;
      return false;
    } else {
      return true;
    }
  }

  if (!HttpLib::IsOnline()) {
    ptr->ErrorMsg = u8"Bad network connection.";
    ptr->ErrorCode = ImageInfo::NetErr;
    return false;
  }

  if (m_Data) {
    m_Data->find(u8"Api")->second.getValueString().clear();
    m_Data->find(u8"Unused")->second.clearA();
    m_Data->find(u8"Used")->second.clearA();
  } else {
    m_Data = new YJson {
      YJson::O {
        {u8"Api"s,        YJson::String},
        {u8"Unused"s,     YJson::Array},
        {u8"Used"s,       YJson::Array},
        {u8"Blacklist"s,  YJson::Array}
      }
    };
  }

  m_Data->find(u8"Api")->second = apiUrl;
  if (DownloadUrl(apiUrl)) {
    return true;
  }

  ptr->ErrorMsg = u8"Bad data has been downloaded.";
  ptr->ErrorCode = ImageInfo::DataErr;
  return false;
}

ImageInfoEx Wallhaven::GetNext()
{
  // https://w.wallhaven.cc/full/1k/wallhaven-1kmx19.jpg

  ImageInfoEx ptr(new ImageInfo);

  if (!CheckData(ptr)) {
    m_Data->toFile(m_DataPath);
    return ptr;
  }

  auto& val = m_Data->find(u8"Unused")->second;
  if (val.emptyA()) {
    YJson::swap(val, m_Data->find(u8"Used")->second);
    std::vector<std::u8string> temp;
    for (auto& i : val.getArray()) {
      temp.emplace_back(std::move(i.getValueString()));
    }
    std::mt19937 g(std::random_device{}());
    std::shuffle(temp.begin(), temp.end(), g);
    val.assignA(std::move(temp).begin(), std::move(temp).end());
  }
  std::u8string name = val.backA().getValueString();
  if (name.length() == 6) {
    if (!IsPngFile(name)) {
      ptr->ErrorMsg = u8"Can't get filetype.";
      ptr->ErrorCode = ImageInfo::NetErr;
      return ptr;
    }
  }
  ptr->ImagePath = (GetImageDir() / name).u8string();
  ptr->ImageUrl =
      u8"https://w.wallhaven.cc/full/"s + name.substr(10, 2) + u8"/"s + name;
  m_Data->find(u8"Used")->second.append(name);
  val.popBackA();
  m_Data->toFile(m_DataPath);
  ptr->ErrorCode = ImageInfo::NoErr;
  return ptr;
}

bool Wallhaven::IsWallhavenFile(std::string name)
{
  std::regex pattern("^wallhaven-[0-9a-z]{6}\\.(png|jpg)$", std::regex::icase);
  return std::regex_match(name, pattern);
}

void Wallhaven::Dislike(const std::u8string& sImgPath)
{
  fs::path img = sImgPath;
  if (!img.has_filename()) return;
  std::u8string m_FileName(img.filename().u8string());
  if (!IsWallhavenFile(Utf8AsString(m_FileName)))
    return;
  
  auto& data =*m_Data;
  data[u8"Unused"].removeByValA(m_FileName);
  data[u8"Used"].removeByValA(m_FileName);
  data[u8"Blacklist"].append(m_FileName);
  data.toFile(m_DataPath);
}

void Wallhaven::UndoDislike(const std::u8string& sImgPath)
{
  fs::path path = sImgPath;
  if (!path.has_filename())
    return;

  auto m_FileName = path.filename().u8string();
  if (!IsWallhavenFile(Utf8AsString(m_FileName)))
    return;

  auto& data =*m_Data;
  data[u8"Blacklist"].removeByValA(m_FileName);
  data[u8"Used"].append(m_FileName);
  data.toFile(m_DataPath);
}

fs::path Wallhaven::GetImageDir() const
{
  return GetCurInfo()[u8"Directory"].getValueString();
}

void Wallhaven::SetCurDir(const std::u8string& str)
{
  GetCurInfo()[u8"Directory"].setText(str);
  SaveSetting();
}

void Wallhaven::SetJson(bool update)
{
  SaveSetting();
  if (!update)
    return;
  m_Data->find(u8"Api")->second.getValueString().clear();
  m_Data->toFile(m_DataPath);
}

size_t Wallhaven::DownloadUrl(const std::u8string& mainUrl) {
  size_t m_TotalDownload = 0;
  std::vector<std::u8string> m_Array;
  auto& m_BlackArray = m_Data->find(u8"Blacklist")->second;

  auto const first = 
    GetCurInfo()[u8"StartPage"].getValueInt();
  auto const last = m_Setting[u8"PageSize"].getValueInt() + first;

  const auto url = Utf8AsString(mainUrl);
  std::unique_ptr<HttpLib> clt;

  auto Get = [&clt, &url](int i){
    auto curUrl = (i == 1 ? url : url + "&page=" + std::to_string(i));
    if (!clt)
      clt = std::unique_ptr<HttpLib>(new HttpLib(std::move(curUrl)));
    else
      clt->SetUrl(std::move(curUrl));
    return clt->Get();
  };

  if (mainUrl.substr(20, 4) == u8"/api") {
    for (size_t n = first; n != last; ++n) {
      auto res = Get(n);
      if (res->status != 200)
        break;
      YJson root(res->body.begin(), res->body.end());
      YJson& data = root[u8"data"sv];
      for (auto& i : data.getArray()) {
        std::u8string name =
            i.find(u8"path")->second.getValueString().substr(31);
        if (m_BlackArray.findByValA(name) == m_BlackArray.endA()) {
          m_Array.emplace_back(name);
          ++m_TotalDownload;
        }
      }
    }
  } else {  // wallhaven-6ozrgw.png
    const std::regex pattern("<li><figure.*?data-wallpaper-id=\"(\\w{6})\"");
    const auto& blackList = m_BlackArray.getArray();
    auto cmp = [](const YJson& i, const std::u8string_view& name) -> bool {
      return i.getValueString().find(name) != std::u8string::npos;
    };
    std::cregex_iterator end;
    for (size_t n = first; n < last; ++n) {
      auto res = Get(n);
      if (res->status != 200)
        break;
      std::string_view body = res->body;
      std::cregex_iterator iter(body.data(), body.data() + body.size(), pattern);
      while (iter != end) {
        const auto& i_ = iter->str(1);
        std::u8string_view i(reinterpret_cast<const char8_t*>(i_.data()),
                              i_.size());
        if (std::find_if(blackList.begin(), blackList.end(), std::bind(cmp,
          std::placeholders::_1, std::ref(i))) == blackList.end() && std::find_if(m_Array.begin(), m_Array.end(), [&i](const YJson& j) -> bool {
            return i == j.getValueString();}) == m_Array.end()) {
          m_Array.emplace_back(i);
          ++m_TotalDownload;
        }
        ++iter;
      }
    }
  }
  if (m_TotalDownload) {
    std::mt19937 g(std::random_device{}());
    std::shuffle(m_Array.begin(), m_Array.end(), g);
    m_Data->find(u8"Unused")->second.assignA(m_Array.begin(), m_Array.end());
  }

  m_Data->toFile(m_DataPath);
  return m_TotalDownload;
}

