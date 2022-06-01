#include "apiclass.hpp"

namespace WallClass {

class Native: public WallBase {
private:
    size_t GetFileCount() {
        size_t m_iCount = 0;
        if (!std::filesystem::exists(m_ImageDir) || !std::filesystem::is_directory(m_ImageDir)) return m_iCount;
        for (auto& iter : std::filesystem::directory_iterator(m_ImageDir)) {
            if (!std::filesystem::is_directory(iter.status()) && Wallpaper::IsImageFile(iter.path())) {
                ++m_iCount;
            }
        }
        return m_iCount;
    }
public:
    explicit Native(const std::filesystem::path& picHome): WallBase(picHome) {
        InitBase();
    }
    virtual ~Native() {
        delete m_Setting;
    }
    virtual ImageInfoEx GetNext() override {
        ImageInfoEx ptr(new std::vector<std::u8string>);
        if (!std::filesystem::exists(m_ImageDir) || !std::filesystem::is_directory(m_ImageDir)) return ptr;

        size_t m_Toltal = GetFileCount(), m_Index = 0, m_ToGet;
        while (!m_RandomQue.empty()) {
            if (m_RandomQue.back() < m_Toltal) {
                m_ToGet = m_RandomQue.back();
                m_RandomQue.pop_back();
                break;
            }
        }

        if (m_RandomQue.empty()) {
            if (m_Toltal < 20) {
                for (int i = 0; i < m_Toltal; i++)
                    m_RandomQue.push_back(i);
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(m_RandomQue.begin(), m_RandomQue.end(), g);
            } else {
                std::mt19937 generator(std::random_device{}());
                auto pf = std::uniform_int_distribution<size_t>(0, m_Toltal-1);
                for (int i=0; i<20; ++i) {
                    auto temp = pf(generator);
                    if (std::find(m_RandomQue.begin(), m_RandomQue.end(), temp) != m_RandomQue.end())
                        temp = pf(generator);
                    m_RandomQue.push_back(temp);
                }
                std::cout << std::endl;
            }
            m_ToGet = m_RandomQue.back();
            m_RandomQue.pop_back();
        }

        if (!m_Toltal) return ptr;

        for (auto& iter : std::filesystem::directory_iterator(m_ImageDir))
        {
            if (!std::filesystem::is_directory(iter.status())) {
                const auto& path = iter.path();
                if (Wallpaper::IsImageFile(path)) {
                    if (m_Index++ == m_ToGet) {
                        ptr->push_back(path.u8string());
                        break;
                    }
                }
            }
        }
        return ptr;
    }
    virtual bool LoadSetting() override {
        if (std::filesystem::exists(m_SettingPath)) {
            m_Setting = new YJson(m_SettingPath, YJson::UTF8);
            m_ImageDir = m_Setting->find(u8"imgdirs")->second.beginA()->getValueString();
            return true;
        }
        return false;
    }
    virtual bool WriteDefaultSetting() override {
        using namespace std::literals;
        m_ImageDir = m_HomePicLocation;
        m_Setting = new YJson(YJson::O {
            { u8"imgdirs"sv, { m_ImageDir }},
            { u8"random"sv, true},
            { u8"recursion"sv, false}
        });
        m_Setting->toFile(m_SettingPath);
        return true;
    }
    virtual void Dislike(const std::filesystem::path& img) override {}
    virtual void SetCurDir(const std::filesystem::path& str) override {
        m_ImageDir = str;
        auto& li = m_Setting->find(u8"imgdirs")->second;
        li.beginA()->setText(str);
        m_Setting->toFile(m_SettingPath);
        m_RandomQue.clear();
    }
    virtual const void* GetDataByName(const char* key) const override {
        if (!strcmp(key, "m_Setting")) {
            return &m_Setting;
        } else {
            return nullptr;
        }
    }
private:
    const char m_SettingPath[12] { "Native.json" };
    YJson* m_Setting;
    std::vector<size_t> m_RandomQue;
};

}
