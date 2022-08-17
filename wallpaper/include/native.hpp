#include <numeric>
#include <set>

#include "apiclass.hpp"

namespace WallClass
{

class Native : public WallBase
{
  private:
    size_t GetFileCount()
    {
        size_t m_iCount = 0;
        if (!std::filesystem::exists(m_ImageDir) || !std::filesystem::is_directory(m_ImageDir))
            return m_iCount;
        for (auto &iter : std::filesystem::directory_iterator(m_ImageDir))
        {
            if (!std::filesystem::is_directory(iter.status()) && Wallpaper::IsImageFile(iter.path()))
            {
                ++m_iCount;
            }
        }
        return m_iCount;
    }

    bool GetFileList()
    {
        size_t m_Toltal = GetFileCount(), m_Index = 0;
        if (!m_Toltal)
            return false;
        std::vector<size_t> numbers;
        if (m_Toltal < 50)
        {
            numbers.resize(m_Toltal);
            std::iota(numbers.begin(), numbers.end(), 0);
        }
        else
        {
            std::set<size_t> already;
            std::mt19937 g(std::random_device{}());
            auto pf = std::uniform_int_distribution<size_t>(0, m_Toltal - 1);
            for (int i = 0; i < 50; ++i)
            {
                auto temp = pf(g);
                while (already.find(temp) != already.end())
                    temp = pf(g);
                already.insert(temp);
                numbers.push_back(temp);
            }
            std::sort(numbers.begin(), numbers.end());
        }

        auto target = numbers.cbegin();
        for (auto &iter : std::filesystem::directory_iterator(m_ImageDir))
        {
            std::filesystem::path path = iter.path();
            if (!std::filesystem::is_directory(iter.status()) && Wallpaper::IsImageFile(path))
            {
                if (*target == m_Index)
                {
                    m_FileList.emplace_back(path.u8string());
                    ++target;
                }
                ++m_Index;
            }
        }

        std::mt19937 g(std::random_device{}());
        std::shuffle(m_FileList.begin(), m_FileList.end(), g);
        return true;
    }

  public:
    explicit Native(const std::filesystem::path &picHome) : WallBase(picHome)
    {
        InitBase();
    }
    virtual ~Native()
    {
        delete m_Setting;
    }
    virtual ImageInfoEx GetNext() override
    {
        ImageInfoEx ptr(new std::vector<std::u8string>);

        while (!m_FileList.empty() && !std::filesystem::exists(m_FileList.back()))
        {
            m_FileList.pop_back();
        }

        if (m_FileList.empty() && !GetFileList())
            return ptr;

        ptr->push_back(std::move(m_FileList.back()));
        m_FileList.pop_back();
        return ptr;
    }
    virtual bool LoadSetting() override
    {
        if (std::filesystem::exists(m_SettingPath))
        {
            m_Setting = new YJson(m_SettingPath, YJson::UTF8);
            m_ImageDir = m_Setting->find(u8"imgdirs")->second.beginA()->getValueString();
            return true;
        }
        return false;
    }
    virtual bool WriteDefaultSetting() override
    {
        using namespace std::literals;
        m_ImageDir = m_HomePicLocation;
        m_Setting = new YJson(YJson::O{{u8"imgdirs"sv, {m_ImageDir}}, {u8"random"sv, true}, {u8"recursion"sv, false}});
        m_Setting->toFile(m_SettingPath);
        return true;
    }
    virtual void Dislike(const std::filesystem::path &img) override
    {
    }
    virtual void UndoDislike(const std::filesystem::path &path) override
    {
    }
    virtual void SetCurDir(const std::filesystem::path &str) override
    {
        m_ImageDir = str;
        auto &li = m_Setting->find(u8"imgdirs")->second;
        li.beginA()->setText(str);
        m_Setting->toFile(m_SettingPath);
        m_FileList.clear();
    }

    virtual std::u8string GetJson() const override
    {
        return m_Setting->toString(false);
    }

    virtual void SetJson(const std::u8string &str) override
    {
        delete m_Setting;
        m_Setting = new YJson(str.begin(), str.end());
        m_Setting->toFile(m_SettingPath);
        //
        //
    }

  private:
    const char m_SettingPath[12]{"Native.json"};
    YJson *m_Setting;
    std::vector<std::u8string> m_FileList;
};

} // namespace WallClass
