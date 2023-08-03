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

YJson WallhavenData::InitData() {
  try {
    return YJson(m_DataPath, YJson::UTF8);
  } catch (std::runtime_error error) {
    return YJson::O {
      {u8"Api"s,        YJson::String},
      {u8"Unused"s,     YJson::Array},
      {u8"Used"s,       YJson::Array},
      {u8"Blacklist"s,  YJson::Array}
    };
  }
}

bool WallhavenData::IsEmpty() const {
  return m_Unused.empty() && m_Used.empty();
}

void WallhavenData::ClearAll() {
  m_Used.clear();
  m_Unused.clear();
}

void WallhavenData::SaveData() {
  m_Data.toFile(m_DataPath);
}

void WallhavenData::DownloadUrl(Range range, Callback callback)
{
  if (range == m_Range) return;

  m_Range = range;
  m_Index = m_Range.front();
  m_Array.clear();

  DownloadAll(std::move(callback));
}


void WallhavenData::HandleResult(const YJson &data)
{
  for (auto& i: data.getArray()) {
    auto name = i.find(u8"path")->second.getValueString().substr(31);
    auto const iter = std::find_if(m_Blacklist.cbegin(), m_Blacklist.cend(), [&name](const YJson& j){
      return j.getValueString() == name;
    });
    if (m_Blacklist.cend() == iter) {
      m_Array.emplace_back(std::move(name));
    }
  }
}

void WallhavenData::DownloadAll(Callback cb)
{
  const std::string url(m_ApiUrl.cbegin(), m_ApiUrl.cend());
  m_Request = std::make_unique<HttpLib>(url + "&page=" + std::to_string(m_Index), true);

  HttpLib::Callback callback = {
    .m_FinishCallback = [this, cb](auto msg, auto res){
      if (msg.empty() && res->status == 200) {
        YJson root(res->body.begin(), res->body.end());
        const auto& data = root[u8"data"];
        if (data.emptyA()) {
          goto handle;
        }
        HandleResult(data);
      }

      if (++m_Index != m_Range.back()) {
        std::thread([this, cb](){ DownloadAll(cb);}).detach();
        return;
      }
handle:
      ImageInfoEx ptr;
      if (!m_Array.empty()) {
        std::mt19937 g(std::random_device{}());
        std::shuffle(m_Array.begin(), m_Array.end(), g);
        m_Mutex.lock();
        m_Unused.assign(m_Array.begin(), m_Array.end());
        m_Mutex.unlock();
      }
      cb();
    }
  };
  m_Request->GetAsync(callback);
}

Wallhaven::Wallhaven(YJson& setting):
  WallBase(InitSetting(setting)),
  m_Data(m_DataMutex, m_DataDir / u8"WallhaveData.json")
{
}

Wallhaven::~Wallhaven()
{
}


bool Wallhaven::IsPngFile(std::u8string& str) {
  // if (!Wallpaper::IsOnline()) return false;
  HttpLib clt(u8"https://wallhaven.cc/api/v1/w/"s + str);
  auto res = clt.Get();
  if (res->status != 200)
    return false;

  Locker locker(m_DataMutex);
  YJson js(res->body.begin(), res->body.end());
  str = js[u8"data"][u8"path"].getValueString().substr(31);
  return true;
}

YJson& Wallhaven::InitSetting(YJson& setting)
{
  if (!setting.isObject()) {
    setting = YJson::O {
      {u8"WallhavenCurrent", u8"最热壁纸"},
      {u8"WallhavenApi", YJson::O {}},
      // {u8"ApiKey", YJson::Null},
      {u8"PageSize", 5},
      {u8"Parameter", YJson::O {
        // {u8"ApiKey", nullptr},
      }}
    };
  }
  if (auto& param = setting[u8"Parameter"]; param.isNull()) {
    param = YJson::Object;
  }
  if (auto& m_ApiObject = setting[u8"WallhavenApi"]; m_ApiObject.emptyO()) {
    std::initializer_list<std::tuple<std::u8string, bool, std::u8string, std::u8string>>
      paramLIst = {
        {u8"最热壁纸"s, true, u8"categories"s, u8"111"s},
        {u8"风景壁纸"s, true, u8"q"s, u8"nature"s},
        {u8"动漫壁纸"s, true, u8"categories"s, u8"010"s},
        {u8"随机壁纸"s, false, u8"sorting"s, u8"random"s},
        {u8"极简壁纸"s, false, u8"q"s, u8"minimalism"s},
        // {u8"鬼刀壁纸"s, false, u8"q"s, u8"ghostblade"s}
    };
    const auto initDir = GetStantardDir(u8"壁纸天堂");

    for (const auto& [i, j, k, l] : paramLIst) {
      auto& item = m_ApiObject.append(YJson::Object, i)->second;
      item[u8"Parameter"] = j ? YJson::O {
        {u8"sorting", u8"toplist"}, {k, l}}: YJson::O {{k, l},};
      item.append(initDir, u8"Directory");
      item.append(1, u8"StartPage");
    }
    SaveSetting();
  }
  return setting;
}

std::u8string Wallhaven::GetApiPathUrl() const
{
  return YJson::joinO(
    GetCurInfo()[u8"Parameter"],
    m_Setting[u8"Parameter"]
  ).urlEncode(u8"https://wallhaven.cc/api/v1/search?"sv);
}

YJson& Wallhaven::GetCurInfo()
{
  return m_Setting[u8"WallhavenApi"][m_Setting[u8"WallhavenCurrent"].getValueString()];
}

bool Wallhaven::CheckData(WallhavenData::Callback callback)
{
  LockerEx locker(m_DataMutex);

  auto const apiUrl = GetApiPathUrl();
  if (m_Data.m_ApiUrl == apiUrl) {
    if (!m_Data.IsEmpty()) return true;
  }

  m_Data.ClearAll();
  m_Data.m_ApiUrl = apiUrl;
  auto const first = GetCurInfo()[u8"StartPage"].getValueInt();
  auto const last = m_Setting[u8"PageSize"].getValueInt() + first;
  locker.unlock();
  m_Data.DownloadUrl({first, last}, callback);
  return false;
}

void Wallhaven::GetNext(Callback callback)
{
  // https://w.wallhaven.cc/full/1k/wallhaven-1kmx19.jpg

  auto handle = [this, callback](){
    ImageInfoEx ptr(new ImageInfo);
    m_DataMutex.lock();
    if (!m_Data.m_Unused.empty()) {
      std::u8string name = m_Data.m_Unused.back().getValueString();
      ptr->ErrorCode = ImageInfo::NoErr;
      ptr->ImagePath = GetCurInfo()[u8"Directory"].getValueString() + u8"/" + name;
      ptr->ImageUrl =
          u8"https://w.wallhaven.cc/full/"s + name.substr(10, 2) + u8"/"s + name;
      m_Data.m_Used.push_back(name);
      m_Data.m_Unused.pop_back();
      m_Data.SaveData();
    } else {
      ptr->ErrorMsg = u8"列表下载失败。";
      ptr->ErrorCode = ImageInfo::NetErr;
    }
    m_DataMutex.unlock();
    callback(ptr);
  };
  if (!CheckData(handle)) return;

  ImageInfoEx ptr(new ImageInfo);
  m_DataMutex.lock();
  if (m_Data.m_Unused.empty()) {
    m_Data.m_Unused.swap(m_Data.m_Used);
    std::vector<std::u8string> temp;
    for (auto& i : m_Data.m_Unused) {
      temp.emplace_back(std::move(i.getValueString()));
    }
    std::mt19937 g(std::random_device{}());
    std::shuffle(temp.begin(), temp.end(), g);
    m_Data.m_Unused.assign(temp.begin(), temp.end());
  }
  m_DataMutex.unlock();
  handle();
  return;
}

std::string Wallhaven::IsWallhavenFile(std::string name)
{
  std::regex pattern("^.*(wallhaven-[0-9a-z]{6}).*\\.(png|jpg)$", std::regex::icase);
  std::smatch result;
  if (!std::regex_match(name, result, pattern)) {
    return std::string();
  }
  return result.str(1) + "." + result.str(2);
}

void Wallhaven::Dislike(std::u8string_view sImgPath)
{
  Locker locker(m_DataMutex);

  fs::path img = sImgPath;
  if (!img.has_filename()) return;
  auto const id = IsWallhavenFile(img.filename().string());
  if (id.empty())
    return;

  YJson const u8Id = StringAsUtf8(id);
  m_Data.m_Unused.remove(u8Id);
  m_Data.m_Used.remove(u8Id);
  m_Data.m_Blacklist.push_back(u8Id);
  m_Data.SaveData();
}

void Wallhaven::UndoDislike(std::u8string_view sImgPath)
{
  Locker locker(m_DataMutex);

  fs::path path = sImgPath;
  if (!path.has_filename())
    return;

  auto const id = IsWallhavenFile(path.filename().string());
  if (id.empty())
    return;

  YJson const u8Id = StringAsUtf8(id);

  m_Data.m_Blacklist.remove(u8Id);
  m_Data.m_Used.push_back(u8Id);
  m_Data.SaveData();
}

void Wallhaven::SetJson(const YJson& json)
{
  WallBase::SetJson(json);

  Locker locker(m_DataMutex);

  m_Data.m_ApiUrl.clear();
  m_Data.SaveData();
}

