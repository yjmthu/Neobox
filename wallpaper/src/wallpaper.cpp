#include "apiclass.hpp"
#include <sysapi.h>
#include <timer.h>

#ifdef _WIN32
#include <wininet.h>
#endif // _WIN32


extern std::unique_ptr<YJson> m_GlobalSetting;
extern const char* m_szClobalSettingFile;
extern void ShowMessage(const std::u8string& title, const std::u8string& text, int type=0);

constexpr char Wallpaper::m_szWallScript[];

bool Wallpaper::DownloadImage(const ImageInfoEx& imageInfo)
{
    if (imageInfo->empty()) return false;
    auto dir = imageInfo->front().substr(0, imageInfo->front().find_last_of(std::filesystem::path::preferred_separator));
    if (!std::filesystem::exists(dir))
        std::filesystem::create_directories(dir);
    if (std::filesystem::exists(imageInfo->front())) {
        if (!std::filesystem::file_size(imageInfo->front()))
            std::filesystem::remove(imageInfo->front());
        else
            return true;
    }
    if (imageInfo->size() != 3) return false;
    try {
    httplib::Client clt(std::string(imageInfo->at(1).begin(), imageInfo->at(1).end()));
    std::ofstream file(std::filesystem::path(imageInfo->front()), std::ios::binary | std::ios::out);
    if (!file.is_open()) return false;
    auto m_fHandleData = [&file](const char* data, size_t length) {
        file.write(data, length);
        return true;
    };
    auto res = clt.Get(reinterpret_cast<const char*>(imageInfo->at(2).c_str()), m_fHandleData);
label:
    if (res->status == 200) {
        file.close();
        return true;
    } else if (res->status == 301 || res->status == 302) {
        file.seekp(std::ios::beg);
        clt.set_follow_location(true);
        res = clt.Get(reinterpret_cast<const char*>(imageInfo->at(2).c_str()), m_fHandleData);
        goto label;
    } else {
        file.close();
        if (std::filesystem::exists(imageInfo->front()))
            std::filesystem::remove(imageInfo->front());
        return false;
    }
    } catch (...) {
        std::cout << "Bad network.";
        return false;
    }
}

Wallpaper::Desktop Wallpaper::GetDesktop()
{
#if defined (_WIN32)
    return Desktop::WIN;
#elif defined (__linux__)
    std::vector<std::string> vec;
    GetCmdOutput<char>("echo $XDG_CURRENT_DESKTOP", vec);
    if (vec[0].find("KDE") != std::string::npos) {
        return Desktop::KDE;
    } else if (vec[0].find("GNOME") != std::string::npos) {
        return Desktop::DDE;
    } else if (vec[0].find("DDE") != std::string::npos) {
        return Desktop::DDE;
    } else if (vec[0].find("XFCE") != std::string::npos) {
        return Desktop::XFCE;
    } else {
        return Desktop::UNKNOWN;
    }
#else
#endif
}

bool Wallpaper::SetWallpaper(const std::filesystem::path& imagePath)
{
    static auto m_DesktopType = GetDesktop();
    if (!std::filesystem::exists(imagePath)) return false;
#if defined (_WIN32)
#ifdef  UNICODE
    std::wstring str = imagePath.wstring();
#else
    std::string str = imagePath.string();
#endif //  UNICDOE
    return ::SystemParametersInfo(
        SPI_SETDESKWALLPAPER,
        UINT(0),
        const_cast<TCHAR*>(str.c_str()),
        SPIF_SENDCHANGE | SPIF_UPDATEINIFILE
    );
#elif defined (__linux__)
    std::ostringstream sstr;
    switch (m_DesktopType) {
    case Desktop::KDE:
        sstr << std::filesystem::current_path() / m_szWallScript << ' ';
        break;
    case Desktop::GNOME:
        sstr << "gsettings set org.gnome.desktop.background picture-uri \"file:";
        break;
    case Desktop::DDE:
        /*
        Old deepin:
        std::string m_sCmd ("gsettings set com.deepin.wrap.gnome.desktop.background picture-uri \"");
    */
        // xrandr|grep 'connected primary'|awk '{print $1}' ======> eDP
        sstr << "dbus-send --dest=com.deepin.daemon.Appearance /com/deepin/daemon/Appearance --print-reply com.deepin.daemon.Appearance.SetMonitorBackground string:\"eDP\" string:\"file://";
        break;
    default:
        std::cout << "不支持的桌面类型；\n";
        return false;
    }
    sstr << imagePath;
    std::string m_sCmd = sstr.str();
    std::cout << imagePath << std::endl << m_sCmd << std::endl;
    return system(m_sCmd.c_str()) == 0;
#endif
}

bool Wallpaper::IsOnline()
{
#ifdef _WIN32
    DWORD flags;
    return InternetGetConnectedState(&flags, 0);
#else
    httplib::Client cli("http://www.msftconnecttest.com");
    try {
        auto res = cli.Get("/connecttest.txt");
        return res->status == 200;
    } catch (...) {
        return false;
    }
#endif
}

Wallpaper::Wallpaper(const std::filesystem::path& picHome)
    : m_Setting(&m_GlobalSetting->find(u8"Wallpaper")->second)
    , m_Wallpaper(nullptr)
    , m_PrevAvailable(false)
    , m_NextAvailable(true)
    , m_PicHomeDir(picHome)
    , m_Timer(new Timer)
{
    SetImageType(m_Setting->find(u8"ImageType")->second.getValueInt());
    m_KeepChange = m_Setting->find(u8"AutoChange")->second.isTrue();
    if (m_KeepChange) {
        m_Timer->StartTimer(m_Setting->find(u8"TimeInterval")->second.getValueInt(),
            std::bind(&Wallpaper::SetSlot, this, 1));
    }
    ReadSettings();
    if (m_Setting->find(u8"FirstChange")->second.isTrue()) {
        WallBase::m_IsWorking = true;
        std::thread([this](){
           for (int i=0; i<30; i++) {
               if (IsOnline()) break;
                std::this_thread::sleep_for(std::chrono::seconds(1));
           }
            if (IsOnline()) {
                SetNext();
                WriteSettings();
            }
            WallBase::m_IsWorking = false;
        }).detach();
    }
}

Wallpaper::~Wallpaper()
{
    delete m_Timer;
    delete m_Wallpaper;
    while (!m_Jobs.empty()) {
        delete m_Jobs.front();
        m_Jobs.pop();
    }
}

bool Wallpaper::IsPrevAvailable()
{
    while (!m_PrevImgs.empty()) {
        if (std::filesystem::exists(m_PrevImgs.back()))
            return true;
        m_PrevImgs.pop_back();
    }
    return false;
}

bool Wallpaper::IsNextAvailable()
{
    // if (setOnline)
        return IsOnline();
    //else
        ;
}

void Wallpaper::SetSlot(int type)
{
    using namespace std::literals;
    if (WallBase::m_IsWorking) {
        ShowMessage(u8"提示", u8"后台正忙，请稍后！");
        return;
    }
    WallBase::m_IsWorking = true;
    std::thread([this, type](){
        switch (type) {
        case -1:
            SetPrevious();
            break;
        case 0:
            RemoveCurrent();
            break;
        case 1:
            SetNext();
            break;
        case 2:
            m_Wallpaper->Update(false);
            WallBase::m_IsWorking = false;
            return;
        case 3:
            m_Wallpaper->Update(true);
            WallBase::m_IsWorking = false;
            return;
        default:
            break;
        }
        WriteSettings();
        WallBase::m_IsWorking = false;
        if (m_KeepChange) {
            m_Timer->Expire();
            m_Timer->StartTimer(m_Setting->find(u8"TimeInterval")->second.getValueInt(),
                std::bind(&Wallpaper::SetSlot, this, 1));
        }
    }).detach();
}

const std::filesystem::path& Wallpaper::GetImageDir() const
{
    return m_Wallpaper->GetImageDir();
}

int Wallpaper::GetTimeInterval() const
{
    return m_Setting->find(u8"TimeInterval")->second.getValueInt();
}

void Wallpaper::SetTimeInterval(int minute)
{
    m_Setting->find(u8"TimeInterval")->second.setValue((double)minute);
    m_GlobalSetting->toFile(m_szClobalSettingFile);
    m_Timer->Expire();
    if (m_KeepChange) {
        m_Timer->Expire();
        m_Timer->StartTimer(m_Setting->find(u8"TimeInterval")->second.getValueInt(),
            std::bind(&Wallpaper::SetSlot, this, 1));
    }
}

bool Wallpaper::SetNext()
{
    if (m_NextImgs.empty()) {
        const ImageInfoEx ptr = m_Wallpaper->GetNext();
        if (ptr->empty() || !DownloadImage(ptr)) return false;
        if (!SetWallpaper(ptr->front()))
            return false;
        if (!m_CurImage.empty()) m_PrevImgs.push_back(m_CurImage);
        m_CurImage = ptr->front();
        return true;
    } else {
        if (SetWallpaper(m_NextImgs.top())) {
            m_PrevImgs.push_back(m_CurImage);
            m_CurImage = m_NextImgs.top();
            m_NextImgs.pop();
            return true;
        } else {
            m_NextImgs.pop();
            return SetNext();
        }
    }
    return false;
}

bool Wallpaper::SetPrevious()
{
    if (m_PrevImgs.empty()) {
        m_PrevAvailable = false;
    } else {
        if (SetWallpaper(m_PrevImgs.back())) {
            m_NextImgs.push(m_CurImage);
            m_CurImage = m_PrevImgs.back();
            m_PrevImgs.pop_back();
            return true;
        } else {
            m_PrevImgs.pop_back();
            return SetPrevious();
        }
    }
    return false;
}

bool Wallpaper::IsImageFile(const std::filesystem::path& filesName)
{
    // BMP, PNG, GIF, JPG
    std::regex pattern(".*\\.(jpg|bmp|gif|jpeg|png)$", std::regex::icase);
    return std::regex_match(filesName.string(), pattern);
}

bool Wallpaper::SetDropFile(const std::filesystem::path& filePath)
{
    if (WallBase::m_IsWorking) return false;
    if (!IsImageFile(filePath))
        return false;
    WallBase::m_IsWorking = true;
    if (!SetWallpaper(filePath))
        return WallBase::m_IsWorking = false;
    if (!m_CurImage.empty()) m_PrevImgs.push_back(m_CurImage);
    m_CurImage = filePath;
    WallBase::m_IsWorking = false;
    return true;
}

bool Wallpaper::RemoveCurrent()
{
    if (m_NextImgs.empty()) {
        if (!SetNext()) return false;
        if (std::filesystem::exists(m_PrevImgs.back()))
            std::filesystem::remove(m_PrevImgs.back());
        m_Wallpaper->Dislike(m_PrevImgs.back());
        m_PrevImgs.pop_back();
        return true;
    } else {
        if (SetWallpaper(m_NextImgs.top())) {
            if (std::filesystem::exists(m_CurImage))
                std::filesystem::remove(m_CurImage);
            m_CurImage = m_NextImgs.top();
            m_NextImgs.pop();
            return true;
        } else {
            m_NextImgs.pop();
            return RemoveCurrent();
        }
    }
    return false;
}

void Wallpaper::ReadSettings()
{
    std::ifstream file("History.txt", std::ios::in);
    if (!file.is_open()) return;
    std::string temp;
    if (std::getline(file, temp)) {
        m_CurImage = temp;
        while (std::getline(file, temp)) {
            m_PrevImgs.emplace_front(temp);
        }
    }
    file.close();
}

void Wallpaper::WriteSettings()
{
    int m_CountLimit = 100;
    std::ofstream file("History.txt", std::ios::out);
    if (!file.is_open()) return;
    if (!m_CurImage.empty()) file << m_CurImage.string() << std::endl;
    for (auto i=m_PrevImgs.rbegin(); i!=m_PrevImgs.rend(); ++i) {
        file << i->string() << std::endl;
        if (!--m_CountLimit) break;
    }
    file.close();
}

void Wallpaper::SetAutoChange(bool flag)
{
    m_Setting->find(u8"AutoChange")->second.setValue(flag);
    m_GlobalSetting->toFile(m_szClobalSettingFile);
    m_Timer->Expire();
    if ((m_KeepChange = flag)) {
        m_Timer->StartTimer(m_Setting->find(u8"TimeInterval")->second.getValueInt(),
            std::bind(&Wallpaper::SetSlot, this, 1));
    }
}


void Wallpaper::SetFirstChange(bool flag)
{
    m_Setting->find(u8"FirstChange")->second.setValue(flag);
    m_GlobalSetting->toFile(m_szClobalSettingFile);
}

void Wallpaper::SetCurDir(const std::filesystem::path& str)
{
    if (!std::filesystem::exists(str))
        std::filesystem::create_directory(str);
    m_Wallpaper->SetCurDir(str);
}

int Wallpaper::GetImageType()
{
    return m_Setting->find(u8"ImageType")->second.getValueInt();
}

bool Wallpaper::SetImageType(int index)
{
    if (WallBase::m_IsWorking) {
        m_Jobs.push(m_Wallpaper);
        m_Wallpaper = nullptr;
    } else {
        delete m_Wallpaper;
    }
    m_Wallpaper = WallBase::GetNewInstance(m_PicHomeDir, index);
    m_Setting->find(u8"ImageType")->second.setValue((int)index);
    m_GlobalSetting->toFile(m_szClobalSettingFile);
    if (index == 1) return true;
    std::thread([this](){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        WallBase *ptr = WallBase::GetNewInstance(m_PicHomeDir, 1);
        for (int i=0; i<7; ++i) {
            Wallpaper::DownloadImage(ptr->GetNext());
        }
        delete ptr;
    }).detach();
    return true;
}

const void* Wallpaper::GetDataByName(const char* key) const
{
    return m_Wallpaper->GetDataByName(key);
}

bool Wallpaper::IsWorking()
{
    return WallBase::m_IsWorking;
}