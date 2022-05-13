#include "apiclass.hpp"

namespace WallClass {

class Native: public WallBase {
private:
    size_t GetFileCount() {
        DIR *dir;
        struct dirent *ptr;
        size_t m_iCount = 0;
        if(!(dir = opendir(m_ImageDir.c_str()))) 
            return m_iCount;
        while((ptr = readdir(dir))) {
            if(strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name,"..") == 0)
                continue;
            if(ptr->d_type == DT_REG && Wallpaper::IsImageFile(ptr->d_name)) {
                m_iCount++;
            }
        }
        closedir(dir);
        return m_iCount;
    }
public:
    explicit Native(): WallBase() {
        InitBase();
    }
    virtual ~Native() {
        delete m_Setting;
    }
    virtual ImageInfo GetNext() {
        ImageInfo ptr(new std::vector<std::string>);
        DIR *dir = nullptr;
        struct dirent *file;
        if(!(dir = opendir(m_ImageDir.c_str()))) return ptr;
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
                std::random_shuffle(m_RandomQue.begin(), m_RandomQue.end(), 
                [](size_t i){return std::rand()%i;});
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
        ptr->push_back(m_ImageDir);
        while((file = readdir(dir))) {
            if(file->d_type == DT_REG && Wallpaper::IsImageFile(file->d_name)) {
                if (m_Index++ == m_ToGet) {
                    ptr->push_back(file->d_name);
                    std::cout << ptr->back() << std::endl;
                    break;
                }
            }
        }
        closedir(dir);
        return ptr;
    }
    virtual bool LoadSetting() {
        if (Wallpaper::PathFileExists(m_SettingPath)) {
            m_Setting = new YJson(m_SettingPath, YJson::UTF8);
            m_ImageDir = m_Setting->find("imgdir")->second.getValueString();

            return true;
        }
        // std::cout << "fffffffff\n";
        return false;
    }
    virtual bool WriteDefaultSetting() {
        m_ImageDir = m_HomePicLocation;
        m_Setting = new YJson(YJson::Object);
        m_Setting->append(m_ImageDir, "imgdir");
        m_Setting->append(true, "random");
        m_Setting->toFile(m_SettingPath);
        return true;
    }
    virtual void Dislike(const std::string& img) {}
    virtual void SetCurDir(const std::string& str) {
        m_Setting->find("imgdir")->second.setText(m_ImageDir = str);
        m_Setting->toFile(m_SettingPath);
    }
    virtual const void* GetDataByName(const char* key) const {
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