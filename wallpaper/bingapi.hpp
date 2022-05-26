#include "apiclass.hpp"

namespace WallClass {

class BingApi: public WallBase {
public:
    explicit BingApi()
        : WallBase()
        , m_Setting(nullptr)
    {
        InitBase();
    }
    virtual ~BingApi() {
        delete m_Setting;
    }
    virtual bool LoadSetting() {
        m_ApiUrl = "https://cn.bing.com";
        m_ImageDir = m_HomePicLocation / u8"必应壁纸";
        m_ImageNameFormat = "\%s \%Y\%m\%d.jpg";
        if (std::filesystem::exists(m_SettingPath)) {
            m_Setting = new YJson(m_SettingPath, YJson::UTF8);
            if (m_Setting->find("today")->second.getValueString() == GetToday("\%Y\%m\%d")) {
                m_ImageDir = m_Setting->find("imgdir")->second.getValueString();
                m_ImageNameFormat = m_Setting->find("imgfmt")->second.getValueString();
                return true;
            } else {
                delete m_Setting;
                m_Setting = nullptr;
                return false;
            }
        }
        return false;
    }
    virtual bool WriteDefaultSetting() {
        // https://cn.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8

        httplib::Client clt(m_ApiUrl);
        auto res = clt.Get("/HPImageArchive.aspx?format=js&idx=0&n=8");
        if (res->status != 200) return false;
        m_Setting = new YJson;
        m_Setting->parse(res->body);
        m_Setting->append(GetToday("\%Y\%m\%d"), "today");
        m_Setting->append(m_ImageDir, "imgdir");
        m_Setting->append(m_ImageNameFormat, "imgfmt");
        m_Setting->toFile(m_SettingPath);
        return true;
    }
    virtual ImageInfoEx GetNext() {
        // https://www.bing.com/th?id=OHR.Yellowstone150_ZH-CN0551084440_UHD.jpg

        auto jsTemp = m_Setting->find("images")->second.find(m_CurImageIndex);
        ImageInfoEx ptr(new std::vector<std::string>);
        ptr->push_back(m_ImageDir / GetImageName(*jsTemp));
        ptr->push_back(m_ApiUrl);
        if (++m_CurImageIndex > 7) m_CurImageIndex = 0;
        ptr->push_back(jsTemp->find("urlbase")->second.getValueString() + "_UHD.jpg");
        return ptr;
    }
    virtual void Dislike(const std::string& img) {
        //
    }
    virtual void SetCurDir(const std::string& str) {
        m_ImageDir = str;
        m_Setting->find("imgdir")->second.setText(str);
        m_Setting->toFile(m_SettingPath);
    }
private:
    std::string m_ImageNameFormat;
    std::string m_ApiUrl;
    const char m_SettingPath[13] { "BingApi.json" };
    YJson* m_Setting;
    unsigned m_CurImageIndex = 0;
    std::string GetToday(const char* fmt) {
        auto t = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), fmt);
        return ss.str();
    }
    std::string GetImageName(YJson& imgInfo) {
        std::string str(m_ImageNameFormat);
        auto pos = str.find("\%s");
        if (pos != std::string::npos) {
            std::string temp =  imgInfo.find("copyright")->second.getValueString();
            temp.erase(temp.find(u8" (© "));
            str.replace(pos, 2, temp);
        }
        auto t = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now() - std::chrono::hours(24 * m_CurImageIndex));
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), str.c_str());
        return ss.str();
    }
};
}
