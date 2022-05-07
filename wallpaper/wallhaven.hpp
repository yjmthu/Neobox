#include "apiclass.hpp"

namespace WallClass {

class Wallhaven: public WallBase {
private:
    // inline static bool SaveJson(const char* path, const boost::json::value& data) {
    //     std::ofstream file(path, std::ios::out);
    //     if (!file.is_open()) return false;;
    //     file << data << std::endl;
    //     file.close();
    //     return true;
    // }
public:
    virtual bool LoadSetting() override {
        try {
            m_Setting = new YJson(m_SettingPath, YJson::UTF8);
            return true;
        } catch (...) {
            return false;
        }
    }
    virtual ImageInfo GetNext() override {
        // https://w.wallhaven.cc/full/1k/wallhaven-1kmx19.jpg

        ImageInfo ptr(new std::vector<std::string>);

        if (m_NeedDownUrl) {
            std::cout << "Start Download Url\n";
            size_t t = DownloadUrl();
            m_NeedDownUrl = false;
            std::cout << t << " Urls Are Downloaded\n";
            if (!t) return ptr;
        }

        auto val = m_Data->find("Unused");
        if (val->empty()) {
            YJson::swap(*val, *m_Data->find("Used"));
        }
        if (val->empty()) {
            std::cout << "No Url\n";
            return ptr;
        }
        std::mt19937 generator(std::random_device{}());
        YJson*& choice = val->find(std::uniform_int_distribution<size_t>(0, val->size()-1)(generator));
        const char* name = choice->getValueString();
        ptr->emplace_back(m_ImageDir);
        ptr->emplace_back(name);
        ptr->emplace_back("https://w.wallhaven.cc");
        ptr->emplace_back("/full/" + std::string(name+10, name+12) + "/" + name);
        m_Data->find("Used")->append(name);
        val->remove(choice);
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
                    m_Data->find("Unused")->removeByVal(m_FileName);
                    m_Data->find("Used")->removeByVal(m_FileName);
                    m_Data->find("Blacklist")->append(m_FileName);
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
        m_Setting->find(m_Setting->find("WallhavenCurrent")->getValueString())->find("Directory")->setText(str);
        m_Setting->toFile(m_SettingPath, YJson::Encode::UTF8, true);
    }
    virtual ~Wallhaven() {
        delete m_Data;
        delete m_Setting;
    }
    virtual std::string GetString() const override {
        return m_Setting->find("WallhavenCurrent")->getValueString();
    }
    virtual int GetInt() const override {
        return m_Setting->find("PageNumber")->getValueInt();
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
        m_Setting->toFile(m_SettingPath, YJson::UTF8, true);
        if (update) {
            GetApiPathUrl();
            m_Data->find("Api")->setText(m_ImageUrl);
            m_Data->find("Used")->clear();
            m_Data->find("Unused")->clear();
            m_NeedDownUrl = true;
        }
    }
private:
    bool WriteDefaultSetting() override {
        delete m_Setting;
        m_Setting = new YJson(u8"{\"WallhavenApi\":{\"Hot\":{\"Parameter\":{ \"sorting\":\"toplist\",\"categories\":111},\"Directory\":\"最热壁纸\"},\"Nature\":{\"Parameter\":{\"sorting\":\"toplist\",\"categories\":100,\"q\":\"nature\"},\"Directory\":\"风景壁纸\"},\"Anime\":{\"Parameter\":{\"categories\":\"010\",\"sorting\":\"toplist\"},\"Directory\":\"动漫壁纸\"},\"Simple\":{\"Parameter\":{\"q\":\"minimalism\"},\"Directory\":\"极简壁纸\"},\"Random\":{\"Parameter\":{\"sorting\":\"random\"},\"Directory\":\"随机壁纸\"}},\"WallhavenCurrent\":\"Hot\",\"PageNumber\":1}");
        for (YJson* i = m_Setting->find("WallhavenApi")->getChild(); i; i = i->getNext()) {
            auto ptr = i->find("Directory");
            ptr->setText(m_HomePicLocation + FILE_SEP_PATH + ptr->getValueString());
        }
        m_Setting->toFile(m_SettingPath, YJson::UTF8, true);
        return true;
    }
    size_t DownloadUrl() {
        size_t m_TotalDownload = 0;
        httplib::Client clt("https://wallhaven.cc");
        auto m_Array = m_Data->find("Unused");
        auto m_BlackArray = m_Data->find("Blacklist");
        for (size_t i=5*(GetInt()-1) + 1, n=i+5; i < n; ++i) {
            std::string url = m_ImageUrl + "&page=" + std::to_string(i);
            auto res = clt.Get(url.c_str());
            if (res->status != 200) break;
            YJson root(res->body);
            YJson& data = root["data"];
            for (YJson* i = data.getChild(); i; i = i->getNext()) {
                const char* name = i->find("path")->getValueString() + 31;
                if ( !m_BlackArray->find(name)) {
                    m_Array->append(name);
                    ++m_TotalDownload;
                }
            }
        }
        m_Data->toFile(m_DataPath);
        return m_TotalDownload;
    }
    void GetApiPathUrl() {
        // m_PageNumber = m_Setting->find("PageNumber")->getValueInt();
        const char* curType = m_Setting->find("WallhavenCurrent")->getValueString();
        // std::cout << "curtype: " << curType << std::endl;
        YJson* val = m_Setting->find("WallhavenApi")->find(curType);
        m_ImageDir = val->find("Directory")->getValueString();
        YJson* param = val->find("Parameter");
        m_ImageUrl = param->urlEncode("/api/v1/search?");
        // for (auto& i: *param) {
        //     m_ImageUrl += i.getKeyString();
        //     m_ImageUrl.push_back('=');
        //     if (i.isString()) {
        //         m_ImageUrl += i.getValueString();
        //     } else if (i.isNumber()) {
        //         m_ImageUrl += std::to_string(i.getValueInt());
        //     } else {
        //         throw std::errc::function_not_supported;
        //     }
        //     m_ImageUrl.push_back('&');
        // }
        // if (str.back() == '&') str.pop_back();
    }
    bool NeedGetImageUrl() {
        if (Wallpaper::PathFileExists(m_DataPath)) {
            try {
                m_Data = new YJson(m_DataPath, YJson::UTF8);
            } catch (...) {
                delete m_Data;
                m_Data = new YJson(YJson::Object);
                m_Data->append(m_ImageUrl, "Api");
                m_Data->append(YJson::Array, "Used");
                m_Data->append(YJson::Array, "Unused");
                m_Data->append(YJson::Array, "Blacklist");
                return true;
            }
            if (m_Data->find("Api")->getValueString() != m_ImageUrl) {
                m_Data->find("Api")->setText(m_ImageUrl);
                m_Data->find("Used")->clear();
                m_Data->find("Unused")->clear();
                return true;
            }
            return false;
        } else {
            m_Data = new YJson(YJson::Object);
            m_Data->append(m_ImageUrl, "Api");
            m_Data->append(YJson::Array, "Used");
            m_Data->append(YJson::Array, "Unused");
            m_Data->append(YJson::Array, "Blacklist");
            return true;
        }
        return true;

    }
    const char m_DataPath[13] { "ImgData.json" };
    const char m_SettingPath[13] { "ApiFile.json" };
    YJson* m_Setting;
    YJson* m_Data;
    std::string m_ImageUrl;
    bool m_NeedDownUrl;
};
}
