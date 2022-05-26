#include "apiclass.hpp"

namespace WallClass {

class DirectApi: public WallBase {
public:
    explicit DirectApi(): WallBase() {
        InitBase();
    }
    virtual ~DirectApi() {}
    virtual bool LoadSetting() {
        using namespace std::literals;
        if (!std::filesystem::exists(m_SettingPath))
            return false;
        m_Setting = new YJson(m_SettingPath, YJson::UTF8);
        auto& data = m_Setting->find("ApiData"sv)->second.find(m_Setting->find("ApiUrl"sv)->second.getValueString())->second;
        m_ApiUrl = data["Url"sv].second.getValueString();
        m_ImageDir = data["Directory"].second.getValueString();
        m_ApiPath = data["Paths"sv].second[data["CurPath"sv].second.getValueInt()].getValueString();
        m_ImageNameFormat = data["ImageNameFormat"sv].second.getValueString();
        return true;
    }
    virtual ImageInfoEx GetNext() {
        return ImageInfoEx(new std::vector<std::string> {
            m_ImageDir / GetImageName(),
            m_ApiUrl,
            m_ApiPath
        });
    }
    virtual void Dislike(const std::string& img) {
        // remove(img.c_str());
    }
    virtual bool WriteDefaultSetting() {
        using namespace std::literals;
        m_ApiUrl = "https://source.unsplash.com";
        m_ApiPath = "/random/2500x1600";
        m_ImageDir = m_HomePicLocation / u8"随机壁纸";
        m_ImageNameFormat = "\%F \%T.jpg";
        m_Setting = new YJson(YJson::O {
            { "ApiUrl"sv, "Unsplash"sv },
            { "ApiData"sv, YJson::O {
                { "Unsplash"sv, YJson::O {
                    { "Url"sv, m_ApiUrl },
                    { "CurPath"sv, 0 },
                    { "Paths"sv, { m_ApiPath } },
                    { "Directory"sv, m_ImageDir },
                    { "ImageNameFormat"sv, m_ImageNameFormat }
                } },
                { "Xiaowai"sv, YJson::O {
                    { "Url"sv, "xiaowai.xyz"sv},
                    { "CurPath"sv, 0 },
                    { "Paths"sv, { "1"sv, "2"sv, "4"sv } },
                    { "Directory"sv, m_HomePicLocation / u8"小歪壁纸" },
                    { "ImageNameFormat"sv, m_ImageNameFormat }
                } }
            } },
        });
        m_Setting->toFile(m_SettingPath);
        return true;
    }
    virtual void SetCurDir(const std::string& str) {
        m_ImageDir = str;
    }
private:
    std::string m_ApiUrl;
    std::string m_ApiPath;
    std::string m_ImageNameFormat;
    std::string GetImageName() {
        auto t = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), m_ImageNameFormat.c_str());
        return ss.str();
    }
    YJson* m_Setting;
    const char m_SettingPath[13] { "ApiFile.json" };
};
}
