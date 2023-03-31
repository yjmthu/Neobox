#include <httplib.h>
#include <systemapi.h>
#include <neotimer.h>
#include <pluginmgr.h>

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

fs::path FileNameFilter(std::u8string& path) {
  std::u8string_view pattern(u8":*?\"<>|");
  std::u8string result;
  result.reserve(path.size());
  for (int count=0; auto c : path) {
    if (pattern.find(c) == pattern.npos) {
      result.push_back(c);
    } else if (c == u8':') {
      // 当路径为相对路径时，可能会有Bug
      if (++count == 1) {
        result.push_back(c);
      }
    }
  }
  path = std::move(result);
  return path;
}

bool Wallpaper::DownloadImage(const ImageInfoEx imageInfo) {
  if (imageInfo->ErrorCode != ImageInfo::NoErr) {
    mgr->ShowMsgbox(u8"出错", imageInfo->ErrorMsg);
    return false;
  }

  // Check image dir and file.
  const auto& filePath = FileNameFilter(imageInfo->ImagePath);
  const auto& dir = filePath.parent_path();

  if (!fs::exists(dir))
    fs::create_directories(dir);
  if (fs::exists(filePath)) {
    if (!fs::file_size(filePath))
      fs::remove(filePath);
    else
      return true;
  }
  if (imageInfo->ImageUrl.empty()) {
    return false;
  }

  if (!HttpLib::IsOnline()) {
    mgr->ShowMsgbox(u8"出错"s, u8"网络异常"s);
    return false;
  }
  HttpLib clt(imageInfo->ImageUrl);
  clt.SetRedirect(1);
  auto res = clt.Get(filePath);
  if (res && res->status == 200) {
    return true;
  } else {
    if (fs::exists(filePath))
      fs::remove(filePath);
    mgr->ShowMsgbox(u8"出错"s, u8"网络异常或文件不能打开！\n文件名："s + filePath.u8string() +
                              u8"\n网址："s + imageInfo->ImageUrl);
    return false;
  }
}

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
  // use preferred separator to prevent win32 api crash.
  imagePath.make_preferred();
#ifdef __linux__
  static auto const m_DesktopType = GetDesktop();
#endif
  if (!fs::exists(imagePath)) {
    mgr->ShowMsgbox(u8"出错", u8"找不到该文件：" + imagePath.u8string());
    return false;
  }
  if (fs::is_directory(imagePath)) {
    mgr->ShowMsgbox(u8"出错", u8"要使用的壁纸不是文件：" + imagePath.u8string());
    return false;
  }
#if defined(_WIN32)
  std::wstring str = imagePath.wstring();
  return ::SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0),
                                 const_cast<WCHAR*>(str.c_str()),
                                 SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
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

Wallpaper::Wallpaper(YJson& settings, std::function<void()> callback)
    :
      m_Settings(settings, callback),
      m_Config(fs::exists(WallBase::m_ConfigPath) ? new YJson(WallBase::m_ConfigPath, YJson::UTF8): new YJson(YJson::Object)),
      m_Wallpaper(nullptr),
      m_Timer(new NeoTimer),
      m_Favorites(WallBase::GetNewInstance(*m_Config, WallBase::FAVORITE)),
      m_BingWallpaper(WallBase::GetNewInstance(*m_Config, WallBase::BINGAPI)) {
  ReadSettings();
  SetImageType(m_Settings.ImageType);
  SetTimeInterval(m_Settings.TimeInterval);
  SetFirstChange(m_Settings.FirstChange.isTrue());
}

Wallpaper::~Wallpaper() {
  delete m_Timer;

  m_ThreadMutex.lock();
  WallBase::ClearInstatnce();
  m_ThreadMutex.unlock();
}

void Wallpaper::SetSlot(OperatorType type) {
  std::thread([this, type]() {
    LockerEx locker(m_ThreadMutex, std::defer_lock);
    if (!locker.try_lock()) {
      mgr->ShowMsgbox(u8"提示", u8"后台正忙，请稍后！");
      return;
    }
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
    WriteSettings();
  }).detach();
}

void Wallpaper::SetTimeInterval(int minute) {
  if (static_cast<int>(m_Settings.TimeInterval) != minute) {
    m_Settings.TimeInterval = minute;
    m_Settings.SaveData();
  }
  m_Timer->Expire();
  if (!m_Settings.AutoChange.isTrue())
    return;
  m_Timer->StartTimer(minute, std::bind(&Wallpaper::SetSlot, this, OperatorType::Next));
}

std::filesystem::path Wallpaper::Url2Name(const std::u8string& url)
{
  std::filesystem::path imagePath = m_Settings.DropDir;
  if (imagePath.is_relative()) {
    imagePath = fs::absolute(imagePath);
    imagePath.make_preferred();
    m_Settings.DropDir = imagePath.u8string();
    m_Settings.SaveData();
  }
  // }
  auto const iter = url.rfind(u8'/') + 1;
  if (m_Settings.DropImgUseUrlName.isTrue()) {
    // url's separator is the '/'.
    imagePath /= url.substr(iter);
  } else {
    const auto& fmt = m_Settings.DropNameFmt;
    auto const extension = url.rfind(u8'.');                       // all url is img file
    auto utc = std::chrono::system_clock::now();

    auto const fmtedName = std::vformat(
      Utf82WideString(fmt + url.substr(extension)),
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
  UpdateRegString(false);

  m_DataMutex.lock();
  auto const ok = m_NextImgs.empty();
  m_DataMutex.unlock();

  if (ok) {
    PushBack(m_Wallpaper->GetNext());
  } else {
    MoveRight();
  }
}

void Wallpaper::UnSetNext() {
  UpdateRegString(true);

  LockerEx locker(m_DataMutex);
  if (!m_PrevImgs.empty()) {
    auto prev = std::move(m_PrevImgs.back());
    m_PrevImgs.pop_back();
    locker.unlock();
    if (!SetWallpaper(prev)) {
      UnSetNext();
      return;
    }
    locker.lock();
    m_NextImgs.push(m_CurImage);
    m_CurImage = std::move(prev);
    m_CurImage.make_preferred();
  }
}

void Wallpaper::UnSetDislike() {
  UpdateRegString(false);

  LockerEx locker(m_DataMutex);
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
  m_PrevImgs.push_back(m_CurImage);
  m_CurImage = std::move(back.second);
  m_CurImage.make_preferred();
  m_Wallpaper->UndoDislike(m_CurImage.u8string());
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
  UpdateRegString(false);

  Locker locker(m_DataMutex);
  if (m_Wallpaper != m_Favorites) {
    m_Favorites->UndoDislike(m_CurImage.u8string());
  }
}

void Wallpaper::UnSetFavorite() {
  UpdateRegString(false);

  LockerEx locker(m_DataMutex);
  if (m_Wallpaper != m_Favorites) {
    m_Favorites->Dislike(m_CurImage.u8string());
    return;
  } else {
    locker.unlock();
    SetDislike();
  }
}

const Wallpaper::String Wallpaper::m_ImgNamePattern {
#ifdef _WIN32
  L""
#endif
  ".*\\.(jpg|bmp|gif|jpeg|png)$"
};

bool Wallpaper::IsImageFile(const fs::path& filesName) {
  // BMP, PNG, GIF, JPG
  std::basic_regex<String::value_type> pattern(m_ImgNamePattern, std::regex::icase);

#ifdef _WIN32
  return std::regex_match(filesName.wstring(), pattern);
#else
  return std::regex_match(filesName.string(), pattern);
#endif
}

void Wallpaper::SetDropFile(std::u8string url) {
  // 在线程外使用 Url2Name 函数
  auto imageName = Url2Name(url);

  std::thread([this, url, imageName]() {
    if (url.starts_with(u8"http")) {
      ImageInfoEx ptr(new ImageInfo{
        imageName.u8string(),
        url,
        {/* ??? */},
        ImageInfo::Errors::NoErr,
      });
      PushBack(ptr);
    } else if (url.starts_with(u8"file")) {
      auto newName { imageName };
      auto oldName { fs::path { url } };
      oldName.make_preferred();

      if (oldName.parent_path() != newName.parent_path()) {
        newName = std::move(oldName);
      } else {                     // don't move wallpaper to the same directory.
        fs::copy(oldName, newName);
      }
      if (SetWallpaper(newName)) {
        m_DataMutex.lock();
        m_PrevImgs.push_back(m_CurImage);
        m_CurImage = std::move(newName);
        m_DataMutex.unlock();
      }
    }
    WriteSettings();
  }).detach();
}

bool Wallpaper::PushBack(ImageInfoEx ptr) {
  if (!DownloadImage(ptr))
    return false;
  if (!SetWallpaper(ptr->ImagePath))
    return false;
  
  Locker locker(m_DataMutex);
  if (!m_CurImage.empty())
    m_PrevImgs.push_back(m_CurImage);
  m_CurImage = ptr->ImagePath;
  m_CurImage.make_preferred();
  return true;
}

bool Wallpaper::MoveRight() {
  LockerEx locker(m_DataMutex);
  auto next = std::move(m_NextImgs.top());
  m_NextImgs.pop();
  locker.unlock();
  if (!SetWallpaper(next)) {
    return false;
  }
  locker.lock();
  m_PrevImgs.push_back(m_CurImage);
  m_CurImage = std::move(next);
  return true;
}

void Wallpaper::SetDislike() {
  UpdateRegString(false);

  LockerEx locker(m_DataMutex);
  auto ok = m_NextImgs.empty();

  locker.unlock();
  if (!((ok && PushBack(m_Wallpaper->GetNext())) || (!ok && MoveRight()))) return;
  locker.lock();

  if (!m_PrevImgs.empty()) {
    auto path = std::move(m_PrevImgs.back());
    m_Wallpaper->Dislike(path.u8string());
    m_PrevImgs.pop_back();
    locker.unlock();
    if (fs::exists(path)) {
      AppendBlackList(path);
    }
  }
}

void Wallpaper::ReadSettings() {
  std::ifstream file("wallpaperData/History.txt", std::ios::in);
  if (file.is_open()) {
    std::string temp;
    if (std::getline(file, temp)) {
      m_CurImage = temp;
      m_CurImage.make_preferred();
      while (std::getline(file, temp)) {
        m_PrevImgs.emplace_front(temp);
      }
    }
    file.close();
  }
  file.open("wallpaperData/Blacklist.txt", std::ios::in);
  if (file.is_open()) {
    std::string key, val;
    while (std::getline(file, key) && std::getline(file, val)) {
      m_BlackList.emplace_back(key, val);
    }
    file.close();
  }
#ifdef _WIN32
  UpdateRegString(false);
#endif
}

#ifdef _WIN32
void Wallpaper::UpdateRegString(bool forward)
{
  Locker locker(m_DataMutex);
  fs::path const curWallpaper = RegReadString(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallPaper");
  if (!curWallpaper.empty() && fs::exists(curWallpaper)) {
    fs::path temp = m_CurImage;
    temp.make_preferred();
    if (temp != curWallpaper) {
      if (forward)
        m_NextImgs.push(temp);
      else
        m_PrevImgs.push_back(temp);
      m_CurImage = curWallpaper.u8string();
    }
  }
}
#endif

void Wallpaper::WriteSettings() {
  int m_CountLimit = 100;
  std::ofstream file("wallpaperData/History.txt", std::ios::out);
  if (!file.is_open())
    return;

  Locker locker(m_DataMutex);
  if (!m_CurImage.empty())
    file << m_CurImage.string() << std::endl;
  for (auto i = m_PrevImgs.rbegin(); i != m_PrevImgs.rend(); ++i) {
    file << i->string() << std::endl;
    if (!--m_CountLimit)
      break;
  }
  file.close();
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
  m_Settings.AutoChange = flag;
  m_Settings.SaveData();
  if (flag) {
    m_Timer->ResetTime(m_Settings.TimeInterval,
                        std::bind(&Wallpaper::SetSlot, this, OperatorType::Next));
  }
}

void Wallpaper::SetFirstChange(bool flag) {
  m_DataMutex.lock();
  m_Settings.FirstChange = flag;
  m_Settings.SaveData();
  m_DataMutex.unlock();

  if (flag) {
    std::thread([this](){
      Locker locker(m_ThreadMutex);
      for (int i =0; i != 300; ++i) {
        std::this_thread::sleep_for(100ms);
        if (WallBase::m_QuitFlag) {
          return;
        }
      }
      SetNext();
      WriteSettings();
    }).detach();
  }
}

bool Wallpaper::SetImageType(int index) {
  LockerEx locker(m_ThreadMutex, std::defer_lock);

  if (!locker.try_lock()) {
    return false;
  }

  if (static_cast<int>(m_Settings.ImageType) != index) {
    m_Settings.ImageType = index;
    m_Settings.SaveData();
  }

  m_Wallpaper = WallBase::GetNewInstance(*m_Config, index);
  return true;
}

// attention: thread maybe working!
