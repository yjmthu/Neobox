#include "apiclass.hpp"
#include "yjson.h"

#include <regex>

namespace WallClass {

class Wallhaven: public WallBase {
private:
    static void IsPngFile(std::string& str) {
        using namespace std::literals;
        httplib::Client clt("https://wallhaven.cc");
        str = "/api/v1/w/"s + str;
        std::cout << str << std::endl;
        auto res = clt.Get(str.c_str());
        YJson js;
        js.parse(res->body);
        // std::cout << js;
        str = js["data"].second["path"].second.getValueString().substr(31);
    }
public:
    virtual bool LoadSetting() override {
        if (!Wallpaper::PathFileExists(m_SettingPath))
            return true;
        try {
            m_Setting = new YJson(m_SettingPath, YJson::UTF8);
            return true;
        } catch (...) {
            return false;
        }
    }
    virtual ImageInfo GetNext() override {
        // https://w.wallhaven.cc/full/1k/wallhaven-1kmx19.jpg
        using namespace std::literals;

        ImageInfo ptr(new std::vector<std::string>);

        if (m_NeedDownUrl) {
            std::cout << "Start Download Url\n"sv;
            size_t t = DownloadUrl();
            m_NeedDownUrl = false;
            std::cout << t << " Urls Are Downloaded\n"sv;
            if (!t) return ptr;
        }

        auto& val = m_Data->find("Unused")->second;
        if (val.emptyA()) {
            YJson::swap(val, m_Data->find("Used")->second);
        }
        if (val.emptyA()) {
            std::cout << "No Url\n";
            return ptr;
        }
        std::mt19937 generator(std::random_device{}());
        auto choice = val.find(std::uniform_int_distribution<size_t>(0, val.sizeA()-1)(generator));
        std::string& name = choice->getValueString();
        if (name.length() == 6) IsPngFile(name);
        else std::cout << "string: " << name << std::endl;
        ptr->emplace_back(m_ImageDir);
        ptr->emplace_back(name);
        ptr->emplace_back("https://w.wallhaven.cc"sv);
        ptr->emplace_back("/full/"s + name.substr(10, 2) + "/"s + name);
        m_Data->find("Used")->second.append(name);
        val.remove(choice);
        m_Data->toFile(m_DataPath);
        return ptr;
    }
    virtual void Dislike(const std::string& img) override {
#if _WIN32
        auto i = std::find(img.rbegin(), img.rend(), '\\').base();
#else
        auto i = std::find(img.rbegin(), img.rend(), '/').base();
#endif
        if (i > img.begin() && i < img.end()) {
            std::string m_FileName(i, img.end());
            if (m_FileName.size() == 20 && 
                std::equal(m_FileName.begin(), m_FileName.begin()+10, "wallhaven-") &&
                    (std::equal(m_FileName.end()-4, m_FileName.end(), ".jpg") || std::equal(m_FileName.end()-4, m_FileName.end(), ".png")) &&
                std::find_if(m_FileName.begin()+10, m_FileName.end()-4, [](char c)->bool{return !isalnum(c);}) == m_FileName.end()-4)
            {
                    m_Data->find("Unused")->second.removeByValA(m_FileName);
                    m_Data->find("Used")->second.removeByValA(m_FileName);
                    m_Data->find("Blacklist")->second.append(m_FileName);
                    m_Data->toFile(m_DataPath);
            } else {
                return;
            }
        } else {
            throw std::errc::not_a_directory;
        }
    }
    explicit Wallhaven()
        : WallBase()
        , m_Setting(nullptr)
        , m_Data(nullptr)
    {
        InitBase();
        GetApiPathUrl();
        m_NeedDownUrl = NeedGetImageUrl();
    }
    virtual void SetCurDir(const std::string& str) override {
        m_ImageDir = str;
        m_Setting->find(m_Setting->find("WallhavenCurrent")->second.getValueString())->second.find("Directory")->second.setText(str);
        m_Setting->toFile(m_SettingPath);
    }
    virtual ~Wallhaven() {
        delete m_Data;
        delete m_Setting;
    }
    virtual std::string GetString() const override {
        return m_Setting->find("WallhavenCurrent")->second.getValueString();
    }
    virtual int GetInt() const override {
        return m_Setting->find("PageNumber")->second.getValueInt();
    }
    virtual const void* GetDataByName(const char* key) const override {
        if (!strcmp(key, "m_Setting")) {
            return &m_Setting;
        } else if (!strcmp(key, "m_Data")) {
            return &m_Data;
        } else if (!strcmp(key, "m_ImageUrl")) {
            return &m_ImageUrl;
        } else {
            return nullptr;
        }
    }
    virtual void Update(bool update) override {
        m_Setting->toFile(m_SettingPath);
        if (update) {
            GetApiPathUrl();
            m_Data->find("Api")->second.setText(m_ImageUrl);
            m_Data->find("Used")->second.clearA();
            m_Data->find("Unused")->second.clearA();
            std::cout << "Need DownloadUrl\n";
            m_NeedDownUrl = true;
        }
    }
private:
    bool WriteDefaultSetting() override {
        using namespace std::literals;
        delete m_Setting;
        m_Setting = new YJson(YJson::Object);
        std::initializer_list<std::tuple<std::string, bool, std::string, std::string>> paramLIst = {
            { u8"最热壁纸"s, true,  "categories"s, "111"s},
            { u8"风景壁纸"s, true,  "q"s,          "nature"s},
            { u8"动漫壁纸"s, true,  "categories"s, "010"s},
            { u8"随机壁纸"s, false, "sorting"s,    "random"s},
            { u8"极简壁纸"s, false, "q"s,          "minimalism"s},
            { u8"鬼刀壁纸"s, false, "q"s,          "ghostblade"s}
        };
        auto& m_ApiObject = m_Setting->append(YJson::Object, "WallhavenApi")->second;
        for (auto& i: paramLIst) {
            std::cout << std::get<0>(i) << std::get<1>(i) << std::get<2>(i) << std::get<3>(i) << std::endl;
            auto& item = m_ApiObject.append(YJson::Object, std::get<0>(i))->second;
            auto& ptr = item.append(YJson::Object, "Parameter")->second;
            if (std::get<1>(i)) {
                ptr.append("toplist", "sorting");
            }
            ptr.append(std::get<3>(i), std::get<2>(i));
            item.append(m_HomePicLocation + FILE_SEP_PATH + std::get<0>(i), "Directory");
        }
        m_Setting->append(std::get<0>(*paramLIst.begin()), "WallhavenCurrent");
        m_Setting->append(1, "PageNumber");
        m_Setting->toFile(m_SettingPath);
        m_Setting->append(YJson::Null, "ApiKey");

        std::cout << "My default json file is: \n" << *m_Setting << std::endl;
        return true;
    }
    size_t DownloadUrl() {
        using namespace std::literals;
        std::cout << "Get next url\n";
        size_t m_TotalDownload = 0;
        httplib::Client clt("https://wallhaven.cc"s);
        auto& m_Array = m_Data->find("Unused")->second.getArray();
        auto& m_BlackArray = m_Data->find("Blacklist")->second;
        if (std::equal(m_ImageUrl.begin(), m_ImageUrl.begin()+4, "/api")) {
            for (size_t i=5*(GetInt()-1) + 1, n=i+5; i < n; ++i) {
                std::string url = m_ImageUrl + "&page=" + std::to_string(i);
                auto res = clt.Get(url.c_str());
                if (res->status != 200) break;
                YJson root; 
                root.parse(res->body);
                YJson& data = root["data"sv].second;
                for (auto& i: data.getArray()) {
                    std::string name = i.find("path")->second.getValueString().substr(31);
                    if (m_BlackArray.findByValA(name) == m_BlackArray.endA()) {
                        m_Array.emplace_back(name);
                        ++m_TotalDownload;
                    }
                }
            }
        } else { // wallhaven-6ozrgw.png
             const std::regex pattern("<li><figure.*?data-wallpaper-id=\"(\\w{6})\"");
             const auto& blackList = m_BlackArray.getArray();
             auto cmp = [](const YJson& i, const std::string& name)->bool { return i.getValueString().find(name) != std::string::npos;};
             std::regex_iterator<std::string_view::const_iterator> end;
             for (size_t i=5*(GetInt()-1) + 1, n=i+5; i < n; ++i) {
                std::string url = (i != 1? m_ImageUrl + "&page=" + std::to_string(i): m_ImageUrl);
                std::cout << url << std::endl;
                auto res = clt.Get(url.c_str());
                if (res->status != 200) break;
                const std::string_view data = res->body;
                std::regex_iterator iter(data.cbegin(), data.cend(), pattern);
                while (iter != end) {
                    const auto& i = iter->str(1);
                    if (std::find_if(blackList.begin(), blackList.end(), std::bind(cmp, std::placeholders::_1, std::ref(i))) == blackList.end() && std::find_if(m_Array.begin(), m_Array.end(), [&i](const YJson& j)->bool{ return j.getValueString() == i;}) == m_Array.end()) {
                        m_Array.emplace_back(i);
                        ++m_TotalDownload;
                    }
                    ++iter;
                }
            }
        }
        m_Data->toFile(m_DataPath);
        return m_TotalDownload;
    }
    void GetApiPathUrl() {
        using namespace std::literals;
        const std::string_view curType = m_Setting->find("WallhavenCurrent")->second.getValueString();
        auto& data = m_Setting->find("WallhavenApi")->second;
        auto& val = data[curType].second;
        m_ImageDir = val.find("Directory")->second.getValueString();
        auto& param = val["Parameter"].second;
        if (param.isObject()) {
            m_ImageUrl = param.urlEncode("/api/v1/search?"sv);
        } else if (param.isString()) {
            m_ImageUrl = param.getValueString();
        } else {
            throw std::runtime_error("Cant find Wallhaven Parameter or Path!"s);
        }
        auto& ApiKey = m_Setting->find("ApiKey")->second;
        if (ApiKey.isString()) {
            m_ImageUrl.append("&apikey="s + ApiKey.getValueString());
        }
    }
    bool NeedGetImageUrl() {
        using namespace std::literals;
        if (Wallpaper::PathFileExists(m_DataPath)) {
            try {
                m_Data = new YJson(m_DataPath, YJson::UTF8);
                if (!m_Data->isObject()) throw nullptr;
                std::array<std::tuple<std::string, YJson::Type, YJson>, 4> lst {
                    std::tuple<std::string, YJson::Type, YJson>
                    { "Used"s, YJson::Array, YJson::Array },
                    { "Unused"s, YJson::Array, YJson::Array },
                    { "Blacklist"s, YJson::Array, YJson::Array },
                    { "Api"s, YJson::String, YJson::String }
                };
                for (auto& [i, j, k]: lst) {
                    auto iter = m_Data->find(i);
                    if (iter == m_Data->endO()) {
                        m_Data->append(std::move(k), i);
                    } else if (iter->second.getType() != j) {
                        YJson::swap(iter->second, k);
                    }
                }
            } catch (...) {
                delete m_Data;
                m_Data = new YJson((YJson::O {
                    { "Api"sv,       m_ImageUrl   },
                    { "Used"sv,      YJson::Array },
                    { "Unused"sv,    YJson::Array },
                    { "Blacklist"sv, YJson::Array }
                }));
                return true;
            }
            if (m_Data->find("Api"sv)->second.getValueString() != m_ImageUrl) {
                m_Data->find("Api"sv)->second.setText(m_ImageUrl);
                m_Data->find("Used"sv)->second.clearA();
                m_Data->find("Unused"sv)->second.clearA();
                return true;
            }
            return false;
        } else {
            m_Data = new YJson(YJson::O {
                {"Api"sv, m_ImageUrl},
                {"Used"sv, YJson::Array},
                {"Unused"sv, YJson::Array},
                {"Blacklist"sv, YJson::Array}
            });
            return true;
        }
        return true;

    }
    const char m_DataPath[13] { "ImgData.json" };
    const char m_SettingPath[18] { "Wallhaven.json" };
    YJson* m_Setting;
    YJson* m_Data;
    std::string m_ImageUrl;
    bool m_NeedDownUrl;
};
}
