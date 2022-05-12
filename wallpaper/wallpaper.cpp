#include "wallpaper/apiclass.hpp"
#include "widgets/speedapp.h"
#include "core/sysapi.h"

#include <string>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

#ifdef _WIN32
#include <Windows.h>
#include <Wininet.h>
#elif defined __linux__
#include <sys/types.h>
#include <sys/io.h>
#include <dirent.h>
#endif

#ifdef _WIN32

inline size_t GetFileSize(const TCHAR* filePath)
{
    WIN32_FIND_DATA fileInfo; 
    HANDLE hFind; 
    DWORD fileSize = 0; 
    hFind = FindFirstFile(filePath, &fileInfo); 
    if(hFind != INVALID_HANDLE_VALUE) 
        fileSize = fileInfo.nFileSizeLow;
    FindClose(hFind);
    return fileSize;
}

#elif defined __linux__

inline size_t GetFileSize(const char* filePath)
{
#if 0
    size_t fileSize = 0;
    FILE* file = fopen(filePath, "r");
    if (file) {
        fileSize = file_length(fileno(file));
        fclose(file);
    }
#else
    struct stat info;
    if (stat(filePath, &info) != 0) return 0;
    size_t fileSize = info.st_size;
#endif
    return fileSize;
}

#endif

// extern int remove (const char *__filename) __THROW;

inline int remove(std::string& path)
{
    return remove(path.c_str());
}

constexpr char Wallpaper::m_szWallScript[16];

bool Wallpaper::DownloadImage(const ImageInfo& imageInfo)
{
    std::string imgFile = (*imageInfo)[0] + FILE_SEP_PATH + (*imageInfo)[1];
    // std::cout << "To save image: " << imgFile << std::endl;
    if (!PathDirExists((*imageInfo)[0]))
        mkdir((*imageInfo)[0].c_str(), 0777);
    if (PathFileExists(imgFile)) {
        if (!GetFileSize(imgFile.c_str()))
            remove(imgFile);
        else
            return true;
    }
    if (imageInfo->size() != 4) return false;
    httplib::Client clt((*imageInfo)[2]);

    std::ofstream file(imgFile, std::ios::binary | std::ios::out);
    if (!file.is_open()) return false;
    auto m_fHandleData = [&file](const char* data, size_t length){
        file.write(data, length);
        return true;
    };
    // std::cout << "Post Get:" << imageInfo->at(3) << std::endl;
    auto res = clt.Get((*imageInfo)[3].c_str(), m_fHandleData);
label:
    if (res->status == 200) {
        // std::cout << "Get Ok\n";
        file.close();
        return true;
    } else if (res->status == 301 || res->status == 302) {
        // std::cout << "Redirect On\n";
        file.seekp(std::ios::beg);
        clt.set_follow_location(true);
        res = clt.Get((*imageInfo)[3].c_str(), m_fHandleData);
        // for (auto& i: res->headers) {
        //     std::cout << i.first << ": " << i.second << std::endl;
        // }
        goto label;
    } else {
        // std::cout << "Get Bad" << res->status << "\n";
        file.close();
        if (PathFileExists(imgFile)) remove(imgFile);
        return false;
    }
}

Wallpaper::Desktop Wallpaper::GetDesktop()
{
#if defined (_WIN32)
    return Desktop::WIN;
#elif defined (__linux__)
    std::vector<std::string> vec;
    GetCmdOutput("echo $XDG_CURRENT_DESKTOP", vec);
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

bool Wallpaper::SetWallpaper(const std::string &imagePath)
{
    static auto m_DesktopType = GetDesktop();
    if (!PathFileExists(imagePath)) return false;
#if defined (_WIN32)
    wxCStrData str = imagePath.c_str();
    return ::SystemParametersInfo(
        SPI_SETDESKWALLPAPER,
        UINT(0),
        const_cast<TCHAR*>(static_cast<const TCHAR*>(str)),
        SPIF_SENDCHANGE | SPIF_UPDATEINIFILE
    );
#elif defined (__linux__)
    std::ostringstream sstr;
    switch (m_DesktopType) {
    case Desktop::KDE:
        char buffer[1024];
        getcwd(buffer, 1024);
        sstr << '\"' << buffer << '/' << m_szWallScript << "\" \"";
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
    sstr << imagePath << '\"';
    std::string m_sCmd = sstr.str();
    return system(m_sCmd.c_str()) == 0;
#endif
}

bool Wallpaper::PathFileExists(const std::string &filePath)
{
    FILE *fp = fopen(filePath.c_str(), "rb");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

bool Wallpaper::PathDirExists(const std::string &dirPath)
{
    DIR *dp = opendir(dirPath.c_str());
    if (dp) {
        closedir(dp);
        return true;
    }
    return false;

}

bool Wallpaper::IsOnline()
{
#ifdef _WIN32
    DWORD flags;
    return InternetGetConnectedState(&flags, 0);
#else
    httplib::Client cli("http://www.msftconnecttest.com");
    auto res = cli.Get("/connecttest.txt");
    return res->status == 200;
#endif
}

Wallpaper::Wallpaper():
    // m_IsWorking(false),
    m_Timer(new QTimer(this)),
    m_Wallpaper(nullptr),
    m_PrevAvailable(false),
    m_NextAvailable(true)
{
    auto m_Setting = m_VarBox->m_Setting->find("Wallpaper");
    SetImageType(m_Setting->find("ImageType")->getValueInt());
    m_Timer->setInterval(m_Setting->find("TimeInterval")->getValueInt() * 60000);
    m_Timer->setSingleShot(true);
    m_KeepChange = m_Setting->find("AutoChange")->isTrue();
    connect(m_Timer, &QTimer::timeout, this, std::bind(&Wallpaper::SetSlot, this, 1));
    connect(this, &Wallpaper::StartTimer, this, [this](bool start){
        // std::cout << "Start Timer\n";
        if (m_Timer->isActive()) m_Timer->stop();
        if (start) m_Timer->start();
    });
    ReadSettings();
    if (m_Setting->find("FirstChange")->isTrue()) {
        // COUT("=======BEGIN1=======");
        WallBase::m_IsWorking = true;
        std::thread([this](){
           for (int i=0; i<30; i++) {
               if (IsOnline()) break;
#ifdef _WIN32
               Sleep(1000);
#elif defined (__linux__)
               sleep(1);
#else
#endif
           }
            if (IsOnline()) {
                SetNext();
                WriteSettings();
            }
            emit StartTimer(m_KeepChange);
            WallBase::m_IsWorking = false;
        }).detach();
    }
}

Wallpaper::~Wallpaper()
{
    if (m_Timer->isActive())
        m_Timer->stop();
    delete m_Wallpaper;
    while (!m_Jobs.empty()) {
        delete m_Jobs.front();
        m_Jobs.pop();
    }
}

bool Wallpaper::IsPrevAvailable()
{
    while (!m_PrevImgs.empty()) {
        if (PathFileExists(m_PrevImgs.back()))
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
    if (WallBase::m_IsWorking) {
        /// DO STH
        QMessageBox::information(nullptr, 
            QStringLiteral("提示"), QStringLiteral("程序正忙，请稍候。"));
    } else {
        // std::cout << "=======BEGIN=======\n";
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
            emit StartTimer(m_KeepChange);
            // std::cout << "=======END=======\n";
        }).detach();
    }
}

const std::string& Wallpaper::GetImageDir() const
{
    return m_Wallpaper->GetImageDir();
}

int Wallpaper::GetTimeInterval() const
{
    return m_VarBox->m_Setting->find("Wallpaper")->find("TimeInterval")->getValueInt();
}

void Wallpaper::SetTimeInterval(int minute)
{
    if (m_Timer->isActive()) {
        m_Timer->start(minute * 60000);
    } else {
        m_Timer->setInterval(minute * 60000);
    }
    m_VarBox->m_Setting->find("Wallpaper")->find("TimeInterval")->setValue((double)minute);
    m_VarBox->SaveSetting();
}

bool Wallpaper::SetNext()
{
    // std::cout << "=====NEXT======\n";
    if (m_NextImgs.empty()) {
        const ImageInfo ptr = m_Wallpaper->GetNext();
        if (ptr->empty() || !DownloadImage(ptr)) return false;
        auto imgFile = (*ptr)[0] + FILE_SEP_PATH + (*ptr)[1];
        if (!SetWallpaper(imgFile))
            return false;
        if (!m_CurImage.empty()) m_PrevImgs.push_back(m_CurImage);
        m_CurImage = imgFile;
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
    std::cout << "=====PREV======\n";
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

bool Wallpaper::IsImageFile(const std::string& fileName)
{
    // BMP, PNG, GIF, JPG
    // auto iter = std::find(fileName.rbegin(), fileName.rend(), '.').base();
    std::string temp = fileName.substr(fileName.find_last_of('.') + 1);
    if (temp.size() != 3) return false;
    for (auto& i: temp) {
        i = toupper(i);
    }
    return temp == "JPG" || temp == "PNG" || temp == "BMP" || temp == "GIF";
}

bool Wallpaper::SetDropFile(const std::string& filePath)
{
    COUT("=====Drop======");
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
    COUT("=====DISLIKE======");
    if (m_NextImgs.empty()) {
        if (!SetNext()) return false;
        if (PathFileExists(m_PrevImgs.back()))
            remove(m_PrevImgs.back());
        m_Wallpaper->Dislike(m_PrevImgs.back());
        m_PrevImgs.pop_back();
        return true;
    } else {
        if (SetWallpaper(m_NextImgs.top())) {
            if (PathFileExists(m_CurImage))
                remove(m_CurImage);
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
        m_CurImage.swap(temp);
        while (std::getline(file, temp)) {
            // std::cout << temp;
            m_PrevImgs.push_front(temp);
        }
    }
    file.close();
}

void Wallpaper::WriteSettings()
{
    int m_CountLimit = 100;
    std::ofstream file("History.txt", std::ios::out);
    if (!file.is_open()) return;
    if (!m_CurImage.empty()) file << m_CurImage << std::endl;
    for (auto i=m_PrevImgs.rbegin(); i!=m_PrevImgs.rend(); ++i) {
        file << *i << std::endl;
        if (!--m_CountLimit) break;
    }
    file.close();
}

void Wallpaper::SetAutoChange(bool flag)
{
    if ((m_KeepChange = flag)) {
        m_Timer->start();
    } else if (m_Timer->isActive()) {
        m_Timer->stop();
    }
    m_VarBox->m_Setting->find("Wallpaper")->find("AutoChange")->setValue(flag);
    m_VarBox->SaveSetting();
}


void Wallpaper::SetFirstChange(bool flag)
{
    m_VarBox->m_Setting->find("Wallpaper")->find("FirstChange")->setValue(flag);
    m_VarBox->SaveSetting();
}

void Wallpaper::SetCurDir(const std::string& str)
{
    if (!PathDirExists(str))
        mkdir(str.c_str(), 0777);
    m_Wallpaper->SetCurDir(str);
}

int Wallpaper::GetImageType()
{
    return m_VarBox->m_Setting->find("Wallpaper")->find("ImageType")->getValueInt();
}

bool Wallpaper::SetImageType(int index)
{
    if (WallBase::m_IsWorking) {
        m_Jobs.push(m_Wallpaper);
        m_Wallpaper = nullptr;
    } else {
        delete m_Wallpaper;
    }
    m_Wallpaper = WallBase::GetNewInstance(index);
    m_VarBox->m_Setting->find("Wallpaper")->find("ImageType")->setValue((int)index);
    m_VarBox->SaveSetting();
    if (index == 1) return true;
    std::thread([](){
#ifdef __WIN32
        Sleep(10000);
#elif defined (__linux__)
        sleep(10);
#else
#endif
        WallBase *ptr = WallBase::GetNewInstance(1);
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

// void Wallpaper::SetValue(const std::string &str, int val)
// {
//     m_Wallpaper->SetValue(str, val);
// }

int Wallpaper::GetInt() const
{
    return m_Wallpaper->GetInt();
}

std::string Wallpaper::GetString() const
{
    return m_Wallpaper->GetString();
}

bool Wallpaper::IsWorking()
{
    return WallBase::m_IsWorking;
}
