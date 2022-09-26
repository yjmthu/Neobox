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

// export module wallpaper6;

using namespace std::literals;

/* export */ class Wallhaven : public WallBase {
 private:
  static bool IsPngFile(std::u8string& str) {
    // if (!Wallpaper::IsOnline()) return false;
    std::u8string body;
    if (HttpLib::Get(u8"https://wallhaven.cc/api/v1/w/"s + str, body) != 200)
      return false;
    YJson js(body.begin(), body.end());
    str = js[u8"data"].second[u8"path"].second.getValueString().substr(31);
    return true;
  }

 public:
  bool LoadSetting() override {
    if (!std::filesystem::exists(m_szSettingPath))
      return false;
    try {
      m_pSetting = new YJson(m_szSettingPath, YJson::UTF8);
      return m_InitOk = true;
    } catch (...) {
      return false;
    }
  }
  ImageInfoEx GetNext() override {
    // https://w.wallhaven.cc/full/1k/wallhaven-1kmx19.jpg

    ImageInfoEx ptr(new ImageInfo);

    if (m_bNeedDownUrl) {
      if (!HttpLib::IsOnline()) {
        ptr->ErrorMsg = u8"Bad network connection.";
        ptr->ErrorCode = ImageInfo::NetErr;
        return ptr;
      }
      size_t t = DownloadUrl();
      m_bNeedDownUrl = false;
      if (!t) {
        ptr->ErrorMsg = u8"Bad data has been downloaded.";
        ptr->ErrorCode = ImageInfo::DataErr;
        return ptr;
      }
    }

    auto& val = m_pData->find(u8"Unused")->second;
    if (val.emptyA()) {
      YJson::swap(val, m_pData->find(u8"Used")->second);
      if (val.emptyA()) {
        ptr->ErrorMsg = u8"No data has been downloaded.";
        ptr->ErrorCode = ImageInfo::DataErr;
        m_bNeedDownUrl = true;
        return ptr;
      } else {
        std::vector<std::u8string> temp;
        for (auto& i : val.getArray()) {
          temp.emplace_back(std::move(i.getValueString()));
        }
        std::mt19937 g(std::random_device{}());
        std::shuffle(temp.begin(), temp.end(), g);
        val.assignA(std::move(temp).begin(), std::move(temp).end());
      }
    }
    std::u8string name = val.backA().getValueString();
    if (name.length() == 6) {
      if (!IsPngFile(name)) {
        ptr->ErrorMsg = u8"Can't get filetype.";
        ptr->ErrorCode = ImageInfo::NetErr;
        return ptr;
      }
    }
    ptr->ImagePath = (m_ImageDir / name).u8string();
    ptr->ImageUrl =
        u8"https://w.wallhaven.cc/full/"s + name.substr(10, 2) + u8"/"s + name;
    m_pData->find(u8"Used")->second.append(name);
    val.popBackA();
    m_pData->toFile(m_szDataPath);
    ptr->ErrorCode = ImageInfo::NoErr;
    return ptr;
  }
  void Dislike(const std::u8string& sImgPath) override {
    std::filesystem::path img = sImgPath;
    if (img.has_filename()) {
      std::u8string m_FileName(img.filename().u8string());
      if (m_FileName.size() == 20 &&
          std::equal(m_FileName.begin(), m_FileName.begin() + 10,
                     u8"wallhaven-") &&
          (std::equal(m_FileName.end() - 4, m_FileName.end(), u8".jpg") ||
           std::equal(m_FileName.end() - 4, m_FileName.end(), u8".png")) &&
          std::find_if(m_FileName.begin() + 10, m_FileName.end() - 4,
                       [](char c) -> bool { return !isalnum(c); }) ==
              m_FileName.end() - 4) {
        m_pData->find(u8"Unused")->second.removeByValA(m_FileName);
        m_pData->find(u8"Used")->second.removeByValA(m_FileName);
        m_pData->find(u8"Blacklist")->second.append(m_FileName);
        m_pData->toFile(m_szDataPath);
      } else {
        return;
      }
    } else {
      throw std::errc::not_a_directory;
    }
  }
  void UndoDislike(const std::u8string& sImgPath) override {
    std::filesystem::path path = sImgPath;
    if (path.has_filename()) {
      std::string m_FileName(path.filename().string());
      std::smatch m_MatchResult;
      std::regex pattern("^wallhaven-[0-9a-z]{6}\\.(png|jpg)$",
                         std::regex::icase);
      if (std::regex_match(m_FileName, m_MatchResult, pattern)) {
        std::string&& str = m_MatchResult.str();
        std::u8string u8str(str.begin(), str.end());
        m_pData->find(u8"Blacklist")->second.removeByValA(u8str);
        m_pData->find(u8"Used")->second.append(u8str);
        m_pData->toFile(m_szDataPath);
      }
    }
  }
  explicit Wallhaven() : WallBase(), m_pSetting(nullptr), m_pData(nullptr) {
    InitBase();
    GetApiPathUrl();
    m_bNeedDownUrl = NeedGetImageUrl();
  }
  void SetCurDir(const std::u8string& str) override {
    m_ImageDir = str;
    m_pSetting
        ->find(m_pSetting->find(u8"WallhavenCurrent")->second.getValueString())
        ->second.find(u8"Directory")
        ->second.setText(str);
    m_pSetting->toFile(m_szSettingPath);
  }
  ~Wallhaven() override {
    delete m_pData;
    delete m_pSetting;
  }

  void SetJson(bool update) override {
    m_pSetting->toFile(m_szSettingPath);
    if (!update)
      return;
    GetApiPathUrl();
    m_pData->find(u8"Api")->second.setText(m_u8strImageUrl);
    m_pData->find(u8"Used")->second.clearA();
    m_pData->find(u8"Unused")->second.clearA();
    m_bNeedDownUrl = update;
  }

  YJson* GetJson() override { return m_pSetting; }

 private:
  bool WriteDefaultSetting() override {
    delete m_pSetting;
    m_pSetting = new YJson(YJson::Object);
    std::initializer_list<
        std::tuple<std::u8string, bool, std::u8string, std::u8string>>
        paramLIst = {{u8"最热壁纸"s, true, u8"categories"s, u8"111"s},
                     {u8"风景壁纸"s, true, u8"q"s, u8"nature"s},
                     {u8"动漫壁纸"s, true, u8"categories"s, u8"010"s},
                     {u8"随机壁纸"s, false, u8"sorting"s, u8"random"s},
                     {u8"极简壁纸"s, false, u8"q"s, u8"minimalism"s},
                     {u8"鬼刀壁纸"s, false, u8"q"s, u8"ghostblade"s}};
    auto& m_ApiObject =
        m_pSetting->append(YJson::Object, u8"WallhavenApi")->second;
    for (auto& i : paramLIst) {
      // std::cout << std::get<0>(i) << std::get<1>(i) << std::get<2>(i) <<
      // std::get<3>(i) << std::endl;
      auto& item = m_ApiObject.append(YJson::Object, std::get<0>(i))->second;
      auto& ptr = item.append(YJson::Object, u8"Parameter")->second;
      if (std::get<1>(i)) {
        ptr.append(u8"toplist", u8"sorting");
      }
      ptr.append(std::get<3>(i), std::get<2>(i));
      item.append((ms_HomePicLocation / std::get<0>(i)).u8string(),
                  u8"Directory");
    }
    m_pSetting->append(std::get<0>(*paramLIst.begin()), u8"WallhavenCurrent"sv);
    m_pSetting->append(1, u8"PageNumber");
    m_pSetting->append(YJson::Null, u8"ApiKey");
    m_pSetting->toFile(m_szSettingPath);

    return true;
  }
  size_t DownloadUrl() {
    std::cout << "Get next url\n";
    size_t m_TotalDownload = 0;
    // auto& m_Array = m_Data->find(u8"Unused")->second.getArray();
    std::vector<std::u8string> m_Array;
    auto& m_BlackArray = m_pData->find(u8"Blacklist")->second;
    size_t i =
        5 * (m_pSetting->find(u8"PageNumber")->second.getValueInt() - 1) + 1;
    const std::string url(m_u8strImageUrl.begin(), m_u8strImageUrl.end());
    if (m_u8strImageUrl.substr(20, 4) == u8"/api") {
      for (size_t n = i + 5; i < n; ++i) {
        std::u8string body;
        int res = HttpLib::Get(
            (i == 1 ? url : url + "&page=" + std::to_string(i)), body);
        if (res != 200)
          break;
        YJson root(body.begin(), body.end());
        YJson& data = root[u8"data"sv].second;
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
      for (size_t n = i + 5; i < n; ++i) {
        std::u8string body;
        auto res = HttpLib::Get(
            (i == 1 ? url : url + "&page=" + std::to_string(i)), body);
        if (res != 200)
          break;
        std::cregex_iterator iter(
            reinterpret_cast<const char*>(body.data()),
            reinterpret_cast<const char*>(body.data()) + body.size(), pattern);
        while (iter != end) {
          const auto& i_ = iter->str(1);
          std::u8string_view i(reinterpret_cast<const char8_t*>(i_.data()),
                               i_.size());
          if (std::find_if(blackList.begin(), blackList.end(),
                           std::bind(cmp, std::placeholders::_1,
                                     std::ref(i))) == blackList.end() &&
              std::find_if(m_Array.begin(), m_Array.end(),
                           [&i](const YJson& j) -> bool {
                             return i == j.getValueString();
                           }) == m_Array.end()) {
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
      m_pData->find(u8"Unused")->second.assignA(m_Array.begin(), m_Array.end());
    }

    m_pData->toFile(m_szDataPath);
    return m_TotalDownload;
  }
  void GetApiPathUrl() {
    const std::u8string_view curType =
        m_pSetting->find(u8"WallhavenCurrent")->second.getValueString();
    auto& data = m_pSetting->find(u8"WallhavenApi")->second;
    auto& val = data[curType].second;
    m_ImageDir = val.find(u8"Directory")->second.getValueString();
    auto& param = val[u8"Parameter"].second;
    if (param.isObject()) {
      m_u8strImageUrl = param.urlEncode(u8"https://wallhaven.cc/api/v1/search?"sv);
    } else if (param.isString()) {
      m_u8strImageUrl = param.getValueString();
    } else {
      throw std::runtime_error("Cant find Wallhaven Parameter or Path!"s);
    }
    auto& ApiKey = m_pSetting->find(u8"ApiKey")->second;
    if (ApiKey.isString()) {
      m_u8strImageUrl.append(u8"&apikey="s + ApiKey.getValueString());
    }
  }
  bool NeedGetImageUrl() {
    if (std::filesystem::exists(m_szDataPath)) {
      try {
        m_pData = new YJson(m_szDataPath, YJson::UTF8);
        if (!m_pData->isObject())
          throw nullptr;
        std::array<std::tuple<std::u8string, YJson::Type, YJson>, 4> lst{
            std::tuple<std::u8string, YJson::Type, YJson>{
                u8"Used"s, YJson::Array, YJson::Array},
            {u8"Unused"s, YJson::Array, YJson::Array},
            {u8"Blacklist"s, YJson::Array, YJson::Array},
            {u8"Api"s, YJson::String, YJson::String}};
        for (auto& [i, j, k] : lst) {
          auto iter = m_pData->find(i);
          if (iter == m_pData->endO()) {
            m_pData->append(std::move(k), i);
          } else if (iter->second.getType() != j) {
            YJson::swap(iter->second, k);
          }
        }
      } catch (...) {
        delete m_pData;
        m_pData = new YJson((YJson::O{{u8"Api"sv, m_u8strImageUrl},
                                     {u8"Used"sv, YJson::Array},
                                     {u8"Unused"sv, YJson::Array},
                                     {u8"Blacklist"sv, YJson::Array}}));
        return true;
      }
      if (m_pData->find(u8"Api"sv)->second.getValueString() != m_u8strImageUrl) {
        m_pData->find(u8"Api"sv)->second.setText(m_u8strImageUrl);
        m_pData->find(u8"Used"sv)->second.clearA();
        m_pData->find(u8"Unused"sv)->second.clearA();
        return true;
      }
      return false;
    } else {
      m_pData = new YJson(YJson::O{{u8"Api"sv, m_u8strImageUrl},
                                  {u8"Used"sv, YJson::Array},
                                  {u8"Unused"sv, YJson::Array},
                                  {u8"Blacklist"sv, YJson::Array}});
      return true;
    }
    return true;
  }
  const char m_szDataPath[13]{"ImgData.json"};
  const char m_szSettingPath[18]{"Wallhaven.json"};
  YJson* m_pSetting;
  YJson* m_pData;
  std::u8string m_u8strImageUrl;
  bool m_bNeedDownUrl;
};
