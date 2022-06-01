#include "apiclass.hpp"

namespace WallClass {

class BingApi: public WallBase {
public:
    explicit BingApi(const std::filesystem::path& picHome)
        : WallBase(picHome)
        , m_Setting(nullptr)
    {
        InitBase();
    }
    virtual ~BingApi() {
        delete m_Setting;
    }
    virtual bool LoadSetting() override {
        m_ApiUrl = u8"https://cn.bing.com";
        m_ImageDir = m_HomePicLocation / u8"必应壁纸";
        m_ImageNameFormat = u8"\%s \%Y\%m\%d.jpg";
        if (std::filesystem::exists(m_SettingPath)) {
            m_Setting = new YJson(m_SettingPath, YJson::UTF8);
            if (m_Setting->find(u8"today")->second.getValueString() == GetToday(u8"\%Y\%m\%d")) {
                m_ImageDir = m_Setting->find(u8"imgdir")->second.getValueString();
                m_ImageNameFormat = m_Setting->find(u8"imgfmt")->second.getValueString();
                return true;
            } else {
                delete m_Setting;
                m_Setting = nullptr;
                return false;
            }
        }
        return false;
    }
    virtual bool WriteDefaultSetting() override {
        // https://cn.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8

        httplib::Client clt(m_ApiUrl);
        auto res = clt.Get("/HPImageArchive.aspx?format=js&idx=0&n=8");
        if (res->status != 200) return false;
        m_Setting = new YJson;
        m_Setting->parse(res->body);
        m_Setting->append(GetToday(u8"\%Y\%m\%d"), u8"today");
        m_Setting->append(m_ImageDir, u8"imgdir");
        m_Setting->append(m_ImageNameFormat, u8"imgfmt");
        m_Setting->toFile(m_SettingPath);
        return true;
    }
    virtual ImageInfoEx GetNext() override {
        // https://www.bing.com/th?id=OHR.Yellowstone150_ZH-CN0551084440_UHD.jpg

        auto jsTemp = m_Setting->find(u8"images")->second.find(m_CurImageIndex);
        ImageInfoEx ptr(new std::vector<std::u8string>);
        ptr->push_back((m_ImageDir / GetImageName(*jsTemp)).u8string());
        ptr->push_back(m_ApiUrl);
        if (++m_CurImageIndex > 7) m_CurImageIndex = 0;
        ptr->push_back(jsTemp->find(u8"urlbase")->second.getValueString() + u8"_UHD.jpg");
        return ptr;
    }
    virtual void Dislike(const std::filesystem::path& img) override {
        //
    }
    virtual void SetCurDir(const std::filesystem::path& str) override {
        m_ImageDir = str;
        m_Setting->find(u8"imgdir")->second.setText(str);
        m_Setting->toFile(m_SettingPath);
    }
private:
    std::u8string m_ImageNameFormat;
    std::u8string m_ApiUrl;
    const char m_SettingPath[13] { "BingApi.json" };
    YJson* m_Setting;
    unsigned m_CurImageIndex = 0;
    std::u8string GetToday(const char8_t* fmt) {
        auto t = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        std::basic_stringstream<char8_t> ss;
        ss << std::put_time(std::localtime(&t), fmt);
        return ss.str();
    }
    std::u8string GetImageName(YJson& imgInfo) {
        std::u8string str(m_ImageNameFormat.begin(), m_ImageNameFormat.end());
        auto pos = str.find(u8"\%s");
        if (pos != std::string::npos) {
            std::u8string temp =  imgInfo.find(u8"copyright")->second.getValueString();
            temp.erase(temp.find(u8" (© "));
            str.replace(pos, 2, temp);
        }
        auto t = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now() - std::chrono::hours(24 * m_CurImageIndex));
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), (const char *)str.c_str());
        std::string temp = ss.str();
        return std::u8string(temp.begin(), temp.end());
    }
};
}
