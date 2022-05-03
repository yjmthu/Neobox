#include "apiclass.hpp"

namespace WallClass {

class DirectApi: public WallBase {
public:
    explicit DirectApi(): WallBase() {
        InitBase();
    }
    virtual ~DirectApi() {}
    virtual bool LoadSetting() {
        return false;
    }
    virtual ImageInfo GetNext() {
        return ImageInfo(new std::vector<std::string> {
            m_ImageDir,
            GetImageName(),
            m_ApiUrl,
            m_ApiPath
        });
    }
    virtual void Dislike(const std::string& img) {
        // remove(img.c_str());
    }
    virtual bool WriteDefaultSetting() {
        m_ApiUrl = "https://source.unsplash.com";
        m_ApiPath = "/random/2500x1600";
        m_ImageDir = m_HomePicLocation + u8"" FILE_SEP_PATH "随机壁纸";
        m_ImageNameFormat = "\%F \%T.jpg";
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
};
}
