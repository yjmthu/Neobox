﻿#include <httplib.h>
#include <yjson.h>

#include <algorithm>
#include <apiclass.hpp>
#include <filesystem>
#include <regex>

namespace WallClass {

class Wallhaven : public WallBase {
 private:
  static void IsPngFile(std::u8string& str) {
    using namespace std::literals;
    httplib::Client clt("https://wallhaven.cc");
    str = u8"/api/v1/w/"s + str;
    auto res = clt.Get(reinterpret_cast<const char*>(str.c_str()));
    YJson js(res->body.begin(), res->body.end());
    str = js[u8"data"].second[u8"path"].second.getValueString().substr(31);
  }

 public:
  virtual bool LoadSetting() override {
    if (!std::filesystem::exists(m_SettingPath)) return false;
    try {
      m_Setting = new YJson(m_SettingPath, YJson::UTF8);
      return true;
    } catch (...) {
      return false;
    }
  }
  virtual ImageInfoEx GetNext() override {
    // https://w.wallhaven.cc/full/1k/wallhaven-1kmx19.jpg
    using namespace std::literals;

    ImageInfoEx ptr(new std::vector<std::u8string>);

    if (m_NeedDownUrl) {
      std::cout << "Start Download Url\n"sv;
      size_t t = DownloadUrl();
      m_NeedDownUrl = false;
      std::cout << t << " Urls Are Downloaded\n"sv;
      if (!t) return ptr;
    }

    auto& val = m_Data->find(u8"Unused")->second;
    if (val.emptyA()) {
      YJson::swap(val, m_Data->find(u8"Used")->second);
      if (val.emptyA()) {
        std::cout << "No Url\n";
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
    if (name.length() == 6) IsPngFile(name);
    ptr->emplace_back((m_ImageDir / name).u8string());
    ptr->emplace_back(u8"https://w.wallhaven.cc"sv);
    ptr->emplace_back(u8"/full/"s + name.substr(10, 2) + u8"/"s + name);
    m_Data->find(u8"Used")->second.append(name);
    val.popBackA();
    m_Data->toFile(m_DataPath);
    return ptr;
  }
  virtual void Dislike(const std::filesystem::path& img) override {
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
        m_Data->find(u8"Unused")->second.removeByValA(m_FileName);
        m_Data->find(u8"Used")->second.removeByValA(m_FileName);
        m_Data->find(u8"Blacklist")->second.append(m_FileName);
        m_Data->toFile(m_DataPath);
      } else {
        return;
      }
    } else {
      throw std::errc::not_a_directory;
    }
  }
  explicit Wallhaven(const std::filesystem::path& picHome)
      : WallBase(picHome), m_Setting(nullptr), m_Data(nullptr) {
    InitBase();
    GetApiPathUrl();
    m_NeedDownUrl = NeedGetImageUrl();
  }
  virtual void SetCurDir(const std::filesystem::path& str) override {
    m_ImageDir = str;
    m_Setting
        ->find(m_Setting->find(u8"WallhavenCurrent")->second.getValueString())
        ->second.find(u8"Directory")
        ->second.setText(str);
    m_Setting->toFile(m_SettingPath);
  }
  virtual ~Wallhaven() {
    delete m_Data;
    delete m_Setting;
  }

  virtual void SetJson(const std::u8string& str) override {
    delete m_Setting;
    m_Setting = new YJson(str.begin(), str.end());
    m_Setting->toFile(m_SettingPath);
    GetApiPathUrl();
    m_Data->find(u8"Api")->second.setText(m_ImageUrl);
    m_Data->find(u8"Used")->second.clearA();
    m_Data->find(u8"Unused")->second.clearA();
    m_NeedDownUrl = true;
  }

  virtual std::u8string GetJson() const override {
    return m_Setting->toU8String(false);
  }

 private:
  bool WriteDefaultSetting() override {
    using namespace std::literals;
    delete m_Setting;
    m_Setting = new YJson(YJson::Object);
    std::initializer_list<
        std::tuple<std::u8string, bool, std::u8string, std::u8string>>
        paramLIst = {{u8"最热壁纸"s, true, u8"categories"s, u8"111"s},
                     {u8"风景壁纸"s, true, u8"q"s, u8"nature"s},
                     {u8"动漫壁纸"s, true, u8"categories"s, u8"010"s},
                     {u8"随机壁纸"s, false, u8"sorting"s, u8"random"s},
                     {u8"极简壁纸"s, false, u8"q"s, u8"minimalism"s},
                     {u8"鬼刀壁纸"s, false, u8"q"s, u8"ghostblade"s}};
    auto& m_ApiObject =
        m_Setting->append(YJson::Object, u8"WallhavenApi")->second;
    for (auto& i : paramLIst) {
      // std::cout << std::get<0>(i) << std::get<1>(i) << std::get<2>(i) <<
      // std::get<3>(i) << std::endl;
      auto& item = m_ApiObject.append(YJson::Object, std::get<0>(i))->second;
      auto& ptr = item.append(YJson::Object, u8"Parameter")->second;
      if (std::get<1>(i)) {
        ptr.append(u8"toplist", u8"sorting");
      }
      ptr.append(std::get<3>(i), std::get<2>(i));
      item.append((m_HomePicLocation / std::get<0>(i)).u8string(),
                  u8"Directory");
    }
    m_Setting->append(std::get<0>(*paramLIst.begin()), u8"WallhavenCurrent"sv);
    m_Setting->append(1, u8"PageNumber");
    m_Setting->append(YJson::Null, u8"ApiKey");
    m_Setting->toFile(m_SettingPath);

    return true;
  }
  size_t DownloadUrl() {
    using namespace std::literals;
    std::cout << "Get next url\n";
    size_t m_TotalDownload = 0;
    httplib::Client clt("https://wallhaven.cc"s);
    // auto& m_Array = m_Data->find(u8"Unused")->second.getArray();
    std::vector<std::u8string> m_Array;
    auto& m_BlackArray = m_Data->find(u8"Blacklist")->second;
    size_t i =
        5 * (m_Setting->find(u8"PageNumber")->second.getValueInt() - 1) + 1;
    if (std::equal(m_ImageUrl.begin(), m_ImageUrl.begin() + 4, "/api")) {
      for (size_t n = i + 5; i < n; ++i) {
        std::string url(
            std::string_view(reinterpret_cast<const char*>(m_ImageUrl.data()),
                             m_ImageUrl.size()));
        url += "&page=" + std::to_string(i);
        auto res = clt.Get(url.c_str());
        if (res->status != 200) break;
        YJson root(res->body.begin(), res->body.end());
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
      std::sregex_iterator end;
      for (size_t n = i + 5; i < n; ++i) {
        std::string url(m_ImageUrl.begin(), m_ImageUrl.end());
        if (i != 1) url += "&page=" + std::to_string(i);
        auto res = clt.Get(url.c_str());
        if (res->status != 200) break;
        std::sregex_iterator iter(res->body.begin(), res->body.end(), pattern);
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
      m_Data->find(u8"Unused")->second.assignA(m_Array.begin(), m_Array.end());
    }

    m_Data->toFile(m_DataPath);
    return m_TotalDownload;
  }
  void GetApiPathUrl() {
    using namespace std::literals;
    const std::u8string_view curType =
        m_Setting->find(u8"WallhavenCurrent")->second.getValueString();
    auto& data = m_Setting->find(u8"WallhavenApi")->second;
    auto& val = data[curType].second;
    m_ImageDir = val.find(u8"Directory")->second.getValueString();
    auto& param = val[u8"Parameter"].second;
    if (param.isObject()) {
      m_ImageUrl = param.urlEncode(u8"/api/v1/search?"sv);
    } else if (param.isString()) {
      m_ImageUrl = param.getValueString();
    } else {
      throw std::runtime_error("Cant find Wallhaven Parameter or Path!"s);
    }
    auto& ApiKey = m_Setting->find(u8"ApiKey")->second;
    if (ApiKey.isString()) {
      m_ImageUrl.append(u8"&apikey="s + ApiKey.getValueString());
    }
  }
  bool NeedGetImageUrl() {
    using namespace std::literals;
    if (std::filesystem::exists(m_DataPath)) {
      try {
        m_Data = new YJson(m_DataPath, YJson::UTF8);
        if (!m_Data->isObject()) throw nullptr;
        std::array<std::tuple<std::u8string, YJson::Type, YJson>, 4> lst{
            std::tuple<std::u8string, YJson::Type, YJson>{
                u8"Used"s, YJson::Array, YJson::Array},
            {u8"Unused"s, YJson::Array, YJson::Array},
            {u8"Blacklist"s, YJson::Array, YJson::Array},
            {u8"Api"s, YJson::String, YJson::String}};
        for (auto& [i, j, k] : lst) {
          auto iter = m_Data->find(i);
          if (iter == m_Data->endO()) {
            m_Data->append(std::move(k), i);
          } else if (iter->second.getType() != j) {
            YJson::swap(iter->second, k);
          }
        }
      } catch (...) {
        delete m_Data;
        m_Data = new YJson((YJson::O{{u8"Api"sv, m_ImageUrl},
                                     {u8"Used"sv, YJson::Array},
                                     {u8"Unused"sv, YJson::Array},
                                     {u8"Blacklist"sv, YJson::Array}}));
        return true;
      }
      if (m_Data->find(u8"Api"sv)->second.getValueString() != m_ImageUrl) {
        m_Data->find(u8"Api"sv)->second.setText(m_ImageUrl);
        m_Data->find(u8"Used"sv)->second.clearA();
        m_Data->find(u8"Unused"sv)->second.clearA();
        return true;
      }
      return false;
    } else {
      m_Data = new YJson(YJson::O{{u8"Api"sv, m_ImageUrl},
                                  {u8"Used"sv, YJson::Array},
                                  {u8"Unused"sv, YJson::Array},
                                  {u8"Blacklist"sv, YJson::Array}});
      return true;
    }
    return true;
  }
  const char m_DataPath[13]{"ImgData.json"};
  const char m_SettingPath[18]{"Wallhaven.json"};
  YJson* m_Setting;
  YJson* m_Data;
  std::u8string m_ImageUrl;
  bool m_NeedDownUrl;
};
}  // namespace WallClass
