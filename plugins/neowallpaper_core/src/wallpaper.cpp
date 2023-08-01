#include <yjson.h>
#include <httplib.h>
#include <stdexcept>
#include <systemapi.h>
#include <neotimer.h>
#include <pluginmgr.h>
#include <download.h>

#ifdef __linux__
#include <unistd.h>
#endif
#include <wallpaper.h>
#include <ranges>
#include <regex>
#include <unordered_set>

namespace fs = std::filesystem;
using namespace std::literals;

// extern std::unordered_set<fs::path> g_UsingFiles;

constexpr char Wallpaper::m_szWallScript[];

Wallpaper::Desktop Wallpaper::GetDesktop() {
#ifdef _WIN32
  return Desktop::WIN;
#elif defined(__linux__)
  auto const ndeEnv = std::getenv("XDG_CURRENT_DESKTOP");
  if (!ndeEnv) {
    return Desktop::UNKNOWN;
  }
  std::string nde = ndeEnv;
  if (nde.find("KDE") != std::string::npos) {
    return Desktop::KDE;
  } else if (nde.find("GNOME") != std::string::npos) {
    return Desktop::DDE;
  } else if (nde.find("DDE") != std::string::npos) {
    return Desktop::DDE;
  } else if (nde.find("XFCE") != std::string::npos) {
    return Desktop::XFCE;
  } else {
    return Desktop::UNKNOWN;
  }
#else
#endif
}

bool Wallpaper::SetWallpaper(fs::path imagePath) {
#ifdef __linux__
  static auto const m_DesktopType = GetDesktop();
#endif
  if (!fs::exists(imagePath) || !fs::is_regular_file(imagePath)) {
    return false;
  }
#if defined(_WIN32)
  std::thread([imagePath]() mutable {
    // use preferred separator to prevent win32 api crash.
    imagePath.make_preferred();
    std::wstring str = imagePath.wstring();
    ::SystemParametersInfoW(
      SPI_SETDESKWALLPAPER, UINT(0),
      const_cast<WCHAR*>(str.c_str()),
      SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
  }).detach();
  return true;
#elif defined(__linux__)
  std::string argStr;
  switch (m_DesktopType) {
    case Desktop::KDE:
      argStr = std::format("var allDesktops = desktops(); print(allDesktops); for (i=0; i < allDesktops.length; i++){{ d = allDesktops[i]; d.wallpaperPlugin = \"org.kde.image\"; d.currentConfigGroup = Array(\"Wallpaper\", \"org.kde.image\", \"General\"); d.writeConfig(\"Image\", \"file://{}\")}}", imagePath.string());
      argStr.push_back('\0');
      if (fork() == 0) {
        execlp(
          "qdbus", "qdbus",
          "org.kde.plasmashell", "/PlasmaShell",
          "org.kde.PlasmaShell.evaluateScript",
          argStr.data(), nullptr
        );
      }
      break;
    case Desktop::GNOME:
      argStr = std::format("\"file:{}\"", imagePath.string());
      argStr.push_back('\0');
      if (fork() == 0) {
        execlp(
          "gsettings", "gsettings",
          "set", "org.gnome.desktop.background", "picture-uri",
          argStr.data(), nullptr
        );
      }
      break;
      // cmdStr = "gsettings set org.gnome.desktop.background picture-uri \"file:" + imagePath.string();
    case Desktop::DDE:
    /*
      Old deepin:
      std::string m_sCmd ("gsettings set
      com.deepin.wrap.gnome.desktop.background picture-uri \"");
    */
      // xrandr|grep 'connected primary'|awk '{print $1}' ======> eDP
      argStr = std::format("string:\"file://\"", imagePath.string());
      argStr.push_back('\0');
      if (fork()) {
        execlp(
          "dbus-send", "dbus-send",
          "--dest=com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance", "--print-reply",
          "com.deepin.daemon.Appearance.SetMonitorBackground", "string:\"eDP\"",
          argStr.data(), nullptr
        );
      }
      break;
    default:
      std::cerr << "不支持的桌面类型；\n";
      return false;
  }
  return true;
#endif
}

Wallpaper::Wallpaper(YJson& settings)
  : m_Settings(settings)
  , m_Config(GetConfigData())
  , m_Wallpaper(nullptr)
  , m_Timer(new NeoTimer)
  , m_Favorites(WallBase::GetNewInstance(*m_Config, WallBase::FAVORITE))
  , m_BingWallpaper(WallBase::GetNewInstance(*m_Config, WallBase::BINGAPI))
{
  ReadBlacklist();
  SetImageType(m_Settings.GetImageType());
  SetTimeInterval(m_Settings.GetTimeInterval());
  SetFirstChange(m_Settings.GetFirstChange());
}

Wallpaper::~Wallpaper() {
  delete m_Timer;

  WallBase::ClearInstatnce();
}

YJson* Wallpaper::GetConfigData()
{
  YJson* data = nullptr;
  try {
    data = new YJson(WallBase::m_ConfigPath, YJson::UTF8);
  } catch (std::runtime_error error) {
    delete data;
    data = new YJson(YJson::Object);
  }

  return data;
}

void Wallpaper::SetSlot(OperatorType type) {
  switch (type) {
    case OperatorType::Next:
      SetNext();
      break;
    case OperatorType::UNext:
      UnSetNext();
      break;
    case OperatorType::Dislike:
      SetDislike();
      break;
    case OperatorType::UDislike:
      UnSetDislike();
      break;
    case OperatorType::Favorite:
      SetFavorite();
      break;
    case OperatorType::UFavorite:
      UnSetFavorite();
      break;
    default:
      break;
  }
  // WriteSettings();
}

void Wallpaper::SetTimeInterval(int minute) {
  if (m_Settings.GetTimeInterval() != minute) {
    m_Settings.SetTimeInterval(minute);
    // m_Settings.SaveData();
  }
  m_Timer->Expire();
  if (!m_Settings.GetAutoChange())
    return;
  m_Timer->StartTimer(minute, std::bind(&Wallpaper::SetSlot, this, OperatorType::Next));
}

std::filesystem::path Wallpaper::Url2Name(const std::u8string& url)
{
  std::filesystem::path imagePath = m_Settings.GetDropDir();
  if (imagePath.is_relative()) {
    imagePath = fs::absolute(imagePath);
    imagePath.make_preferred();
    m_Settings.SetDropDir(imagePath.u8string());
  }
  // }
  auto const iter = url.rfind(u8'/') + 1;
  if (m_Settings.GetDropImgUseUrlName()) {
    // url's separator is the '/'.
    imagePath /= url.substr(iter);
  } else {
    auto fmtStr = m_Settings.GetDropNameFmt();
    auto const extension = url.rfind(u8'.');                       // all url is img file
    auto utc = std::chrono::system_clock::now();

    auto const fmtedName = std::vformat(
      Utf82WideString(fmtStr + url.substr(extension)),
      std::make_wformat_args(
        std::chrono::current_zone()->to_local(utc),
        Utf82WideString(url.substr(iter, extension - iter))
      )
    );
    imagePath.append(fmtedName);
  }

  return imagePath.make_preferred();
}

void Wallpaper::SetNext() {
  m_DataMutex.lock();
  m_PrevImgs.UpdateRegString();
  auto const ok = m_NextImgs.empty();
  m_DataMutex.unlock();

  if (ok) {
    m_Wallpaper->GetNext(
      std::bind(&Wallpaper::PushBack,
        this,
        std::placeholders::_1,
        std::nullopt
      )
    );
  } else {
    MoveRight();
  }
}

void Wallpaper::UnSetNext() {
  LockerEx locker(m_DataMutex);
  m_PrevImgs.UpdateRegString();
  auto prev = m_PrevImgs.GetPrevious();
  if (prev) {
    auto cur = m_PrevImgs.GetCurrent();
    if (cur) {
      m_PrevImgs.EraseCurrent();
      m_NextImgs.push(std::move(*cur));
    }
    locker.unlock();
    SetWallpaper(*prev);
  } else {
    mgr->ShowMsgbox(L"提示", L"当前已经是第一张壁纸！");
  }
}

void Wallpaper::UnSetDislike() {
  LockerEx locker(m_DataMutex);
  m_PrevImgs.UpdateRegString();
  if (m_BlackList.empty())
    return;
  auto back = std::move(m_BlackList.back());
  m_BlackList.pop_back();

  if (!back.second.has_parent_path()) {
    locker.unlock();
    WriteBlackList();
    return;
  }

  auto parent_path = back.second.parent_path();
  if (!fs::exists(parent_path)) {
    fs::create_directories(parent_path);
  }
  fs::rename(back.first, back.second);
  locker.unlock();
  if (!SetWallpaper(back.second))
    return;

  locker.lock();
  m_Wallpaper->UndoDislike(back.second.u8string());
  m_PrevImgs.PushBack(std::move(back.second));
  locker.unlock();
  WriteBlackList();
}

void Wallpaper::ClearJunk() {
  constexpr char8_t junk[] = u8"junk";
  if (fs::exists(junk))
    fs::remove_all(junk);
  if (fs::exists("Blacklist.txt"))
    fs::remove("Blacklist.txt");
  
  m_DataMutex.lock();
  m_BlackList.clear();
  m_DataMutex.unlock();
}

void Wallpaper::SetFavorite() {
  Locker locker(m_DataMutex);
  m_PrevImgs.UpdateRegString();
  if (m_Wallpaper != m_Favorites) {
    auto curImage = m_PrevImgs.GetCurrent();
    if (curImage) {
      m_Favorites->UndoDislike(curImage->u8string());
    }
  }
}

void Wallpaper::UnSetFavorite() {
  LockerEx locker(m_DataMutex);
  m_PrevImgs.UpdateRegString();
  if (m_Wallpaper != m_Favorites) {
    auto curImage = m_PrevImgs.GetCurrent();
    if (curImage) {
      m_Favorites->Dislike(curImage->u8string());
    }
  } else {
    locker.unlock();
    SetDislike();
  }
}

void Wallpaper::SetDropFile(std::queue<std::u8string_view> urls) {

  while (!urls.empty()) {
    std::u8string url(urls.front());
    urls.pop();
    if (!DownloadJob::IsImageFile(url)) {
      continue;
    }
    auto imageName = Url2Name(url);
    if (url.starts_with(u8"http")) {
      ImageInfoEx ptr(new ImageInfo {
        .ImagePath = imageName.u8string(),
        .ImageUrl = url,
        .ErrorCode = ImageInfo::Errors::NoErr,
      });
      PushBack(ptr, std::nullopt);
    } else if (fs::path oldName = url; fs::exists(oldName)) {
      oldName.make_preferred();

      if (oldName.parent_path() != imageName.parent_path()) {
        imageName = std::move(oldName);
      } else {                     // don't move wallpaper to the same directory.
        fs::copy(oldName, imageName);
      }
      if (SetWallpaper(imageName)) {
        m_DataMutex.lock();
        m_PrevImgs.PushBack(std::move(imageName));
        m_DataMutex.unlock();
      }
    }
  }
}

void Wallpaper::PushBack(ImageInfoEx ptr, 
  std::optional<std::function<void()>> callback)
{
  
  DownloadJob::DownloadImage(ptr, [this, ptr, callback](){
    if (!SetWallpaper(ptr->ImagePath))
      return;
    
    LockerEx locker(m_DataMutex);
    m_PrevImgs.PushBack(ptr->ImagePath);

    if (callback) {
      locker.unlock();
      callback->operator()();
    }
  });
}

bool Wallpaper::MoveRight() {
  LockerEx locker(m_DataMutex);
  fs::path next { std::move(m_NextImgs.top()) };
  m_NextImgs.pop();
  locker.unlock();
  if (!SetWallpaper(next)) {
    return false;
  }
  locker.lock();
  m_PrevImgs.PushBack(std::move(next));
  return true;
}

void Wallpaper::SetDislike() {
  m_DataMutex.lock();
  m_PrevImgs.UpdateRegString();
  auto const ok = m_NextImgs.empty();
  m_DataMutex.unlock();

  auto callback = [this](){
    LockerEx locker(m_DataMutex);
    // 取出之前的图片栈顶元素，并将其设置为dislike
    auto curImage = m_PrevImgs.GetPrevious();

    if (!curImage) return ;
    m_PrevImgs.ErasePrevious();
    m_Wallpaper->Dislike(curImage->u8string());
    if (fs::exists(*curImage)) {
      locker.unlock();
      AppendBlackList(*curImage);
      locker.lock();
    }
  };

  if (ok) {
    // 设置一张新的壁纸, 并把之前的壁纸设置为dislike
    m_Wallpaper->GetNext(std::bind(&Wallpaper::PushBack,
      this, std::placeholders::_1, callback));
  } else {
    MoveRight();
    callback();
  }
}

void Wallpaper::ReadBlacklist() {
  std::ifstream file("wallpaperData/Blacklist.txt", std::ios::in);
  if (file.is_open()) {
    std::string key, val;
    while (std::getline(file, key) && std::getline(file, val)) {
      m_BlackList.emplace_back(key, val);
    }
    file.close();
  }
}

void Wallpaper::AppendBlackList(const fs::path& path) {
  constexpr char8_t junk[] = u8"junk";
  if (!fs::exists(junk)) {
    fs::create_directory(junk);
  }

  Locker locker(m_DataMutex);
  auto& back = m_BlackList.emplace_back(junk / path.filename(), path);
  fs::rename(back.second, back.first);

  std::ofstream file("wallpaperData/Blacklist.txt", std::ios::app);
  if (!file.is_open())
    return;
  file << back.first.string() << std::endl << back.second.string() << std::endl;
  file.close();
}

void Wallpaper::WriteBlackList() {
  std::ofstream file("wallpaperData/Blacklist.txt", std::ios::out);
  if (!file.is_open())
    return;
  
  Locker locker(m_DataMutex);
  for (auto& i : m_BlackList) {
    file << i.first.string() << std::endl << i.second.string() << std::endl;
  }
  file.close();
}

void Wallpaper::SetAutoChange(bool flag) {
  m_Timer->Expire();

  Locker locker(m_DataMutex);
  m_Settings.SetAutoChange(flag);
  // m_Settings.SaveData();
  if (flag) {
    m_Timer->ResetTime(
    m_Settings.GetTimeInterval(),
        std::bind(&Wallpaper::SetSlot, this, OperatorType::Next));
  }
}

void Wallpaper::SetFirstChange(bool flag) {
  m_DataMutex.lock();
  m_Settings.SetFirstChange(flag);
  m_DataMutex.unlock();

  if (flag) {
    std::thread([this](){
      for (int i =0; i != 300; ++i) {
        std::this_thread::sleep_for(100ms);
        if (WallBase::m_QuitFlag) {
          return;
        }
      }
      SetNext();
    }).detach();
  }
}

bool Wallpaper::SetImageType(int index) {
  if (m_Settings.GetImageType() != index) {
    m_Settings.SetImageType(index);
    // m_Settings.SaveData();
  }

  m_Wallpaper = WallBase::GetNewInstance(*m_Config, index);
  return true;
}

// attention: thread maybe working!
