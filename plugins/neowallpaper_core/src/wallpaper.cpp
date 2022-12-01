#include <httplib.h>
#include <systemapi.h>
#include <neotimer.h>
#include <neoapp.h>

#include <wallpaper.h>
#include <ranges>
#include <regex>
#include <unordered_set>

extern GlbObject* glb;
namespace fs = std::filesystem;
using namespace std::literals;

extern std::unordered_set<fs::path> g_UsingFiles;

constexpr char Wallpaper::m_szWallScript[];

bool Wallpaper::DownloadImage(const ImageInfoEx imageInfo) {
  if (imageInfo->ErrorCode != ImageInfo::NoErr) {
    glb->glbShowMsgbox(u8"出错", imageInfo->ErrorMsg);
    return false;
  }

  // Check image dir and file.
  const auto& u8FilePath = imageInfo->ImagePath;
  const auto& dir = fs::path(u8FilePath).parent_path();

  if (!fs::exists(dir))
    fs::create_directories(dir);
  if (fs::exists(u8FilePath)) {
    if (!fs::file_size(u8FilePath))
      fs::remove(u8FilePath);
    else
      return true;
  }
  if (imageInfo->ImageUrl.empty()) {
    return false;
  }

  if (!HttpLib::IsOnline()) {
    glb->glbShowMsgbox(u8"出错"s, u8"网络异常"s);
    return false;
  }
  g_UsingFiles.emplace(u8FilePath);
  HttpLib clt(imageInfo->ImageUrl);
  clt.SetRedirect(1);
  auto res = clt.Get(u8FilePath);
  if (res->status == 200) {
    g_UsingFiles.erase(u8FilePath);
    return true;
  } else {
    if (fs::exists(u8FilePath))
      fs::remove(u8FilePath);
    g_UsingFiles.erase(u8FilePath);
    glb->glbShowMsgbox(u8"出错"s, u8"网络异常或文件不能打开！\n文件名："s + u8FilePath +
                              u8"\n网址："s + imageInfo->ImageUrl);
    return false;
  }
}

Wallpaper::Desktop Wallpaper::GetDesktop() {
#ifdef _WIN32
  return Desktop::WIN;
#elif defined(__linux__)
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

bool Wallpaper::SetWallpaper(fs::path imagePath) {
  // use preferred separator to prevent win32 api crash.
  imagePath.make_preferred();

  [[maybe_unused]] static auto m_DesktopType = GetDesktop();
  if (!fs::exists(imagePath)) {
    glb->glbShowMsgbox(u8"出错", u8"找不到该文件：" + imagePath.u8string());
    return false;
  }
  if (fs::is_directory(imagePath)) {
    glb->glbShowMsgbox(u8"出错", u8"要使用的壁纸不是文件：" + imagePath.u8string());
    return false;
  }
#if defined(_WIN32)
  std::wstring str = imagePath.wstring();
  return ::SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0),
                                 const_cast<WCHAR*>(str.c_str()),
                                 SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
#elif defined(__linux__)
  std::ostringstream sstr;
  switch (m_DesktopType) {
    case Desktop::KDE:
      sstr << fs::current_path() / m_szWallScript << ' ';
      break;
    case Desktop::GNOME:
      sstr << "gsettings set org.gnome.desktop.background picture-uri \"file:";
      break;
    case Desktop::DDE:
    /*
      Old deepin:
      std::string m_sCmd ("gsettings set
      com.deepin.wrap.gnome.desktop.background picture-uri \"");
    */
      // xrandr|grep 'connected primary'|awk '{print $1}' ======> eDP
      sstr << "dbus-send --dest=com.deepin.daemon.Appearance "
              "/com/deepin/daemon/Appearance --print-reply "
              "com.deepin.daemon.Appearance.SetMonitorBackground "
              "string:\"eDP\" string:\"file://";
      break;
    default:
      std::cerr << "不支持的桌面类型；\n";
      return false;
  }
  sstr << imagePath;
  std::string m_sCmd = sstr.str();
  return system(m_sCmd.c_str()) == 0;
#endif
}

Wallpaper::Wallpaper(YJson& settings, std::function<void()> callback)
    :  // m_PicHomeDir(GetSpecialFolderPath(CSIDL_MYPICTURES)),
      m_Settings(settings),
      m_Config(fs::exists(WallBase::m_ConfigPath) ? new YJson(WallBase::m_ConfigPath, YJson::UTF8): new YJson(YJson::Object)),
      m_Wallpaper(nullptr),
      SettingsCallback(callback),
      m_Timer(new NeoTimer),
      m_Favorites(WallBase::GetNewInstance(*m_Config, WallBase::FAVORITE)),
      m_BingWallpaper(WallBase::GetNewInstance(*m_Config, WallBase::BINGAPI)) {
  ReadSettings();
  SetImageType(GetImageType());
  SetTimeInterval(GetTimeInterval());
  SetFirstChange(GetFirstChange());
}

Wallpaper::~Wallpaper() {
  delete m_Timer;
  WallBase::ClearInstatnce();
  for (const auto& i: g_UsingFiles ) {
    if (fs::exists(i))
      fs::remove(i);
  }
}

void Wallpaper::SetSlot(int type) {
  if (WallBase::ms_IsWorking) {
    glb->glbShowMsgbox(u8"提示", u8"后台正忙，请稍后！");
    return;
  }
  WallBase::ms_IsWorking = true;
  std::thread([this, type]() {
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
      default:
        break;
    }
    WriteSettings();
    WallBase::ms_IsWorking = false;
    if (GetAutoChange()) {
      m_Timer->Expire();
      m_Timer->StartTimer(GetTimeInterval(),
                          std::bind(&Wallpaper::SetSlot, this, 1));
    }
  }).detach();
}

fs::path Wallpaper::GetImageDir() const {
  return m_Wallpaper->GetImageDir();
}

void Wallpaper::SetTimeInterval(int minute) {
  auto& jsTimeInterval = m_Settings[u8"TimeInterval"];
  if (jsTimeInterval.getValueInt() != minute) {
    jsTimeInterval.setValue(minute);
    SettingsCallback();
  }
  m_Timer->Expire();
  if (!GetAutoChange())
    return;
  m_Timer->Expire();
  m_Timer->StartTimer(minute, std::bind(&Wallpaper::SetSlot, this, 1));
}

std::filesystem::path Wallpaper::GetImageName(const std::u8string& url)
{
  std::filesystem::path imagePath;
  if (m_Settings[u8"DropToCurDir"].isTrue()) {
    imagePath = m_Wallpaper->GetImageDir();
  } else {
    imagePath = m_Settings[u8"DropDir"].getValueString();
  }
  auto iter = std::find(url.crbegin(), url.crend(), '/').base();
  if (m_Settings[u8"DropImgUseUrlName"].isTrue()) {
    // url's separator is the '/'.
    imagePath.append(std::u8string(iter, url.end()));
  } else {
    const auto& fmt = m_Settings[u8"DropINameFmt"].getValueString();
    auto pt = std::find(url.crbegin(), url.crend(), u8'.').base() - 1;
    auto utc = std::chrono::system_clock::now();
    auto fmtedName = std::vformat(
      std::string(fmt.begin(), fmt.end()),
      std::make_format_args(std::chrono::current_zone()->to_local(utc), std::string(iter, pt)));
    imagePath.append(std::u8string(fmtedName.begin(), fmtedName.end()) + std::u8string(pt, url.cend()));
  }
  return imagePath.make_preferred();
}

bool Wallpaper::SetNext() {
  if (!m_NextImgsBuffer.empty()) {
    std::u8string imgUrl = std::move(m_NextImgsBuffer.front());
    m_NextImgsBuffer.pop_front();
    if (imgUrl.starts_with(u8"http")) {
      ImageInfoEx ptr(new ImageInfo{
        GetImageName(imgUrl).u8string(),
        imgUrl,
        std::u8string(),
        ImageInfo::Errors::NoErr,
      });
      if (DownloadImage(ptr)) {
        if (SetWallpaper(ptr->ImagePath)) {
          m_PrevImgs.push_back(m_CurImage);
          m_CurImage = ptr->ImagePath;
          return true;
        }
      }
    } else if (fs::exists(imgUrl)) {
      if (SetWallpaper(imgUrl)) {
        m_PrevImgs.push_back(m_CurImage);
        m_CurImage = imgUrl;
        return true;
      }
    } else if (!m_NextImgsBuffer.empty()) {
      return SetNext();
    }
    return false;
  }
  if (m_NextImgs.empty()) {
    const ImageInfoEx ptr = m_Wallpaper->GetNext();
    if (!DownloadImage(ptr))
      return false;
    if (!SetWallpaper(ptr->ImagePath))
      return false;
    if (!m_CurImage.empty())
      m_PrevImgs.push_back(m_CurImage);
    m_CurImage = ptr->ImagePath;
    m_CurImage.make_preferred();
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

bool Wallpaper::SetPrevious() {
  if (!m_PrevImgs.empty()) {
    if (SetWallpaper(m_PrevImgs.back())) {
      m_NextImgs.push(m_CurImage);
      m_CurImage = std::move(m_PrevImgs.back());
      m_CurImage.make_preferred();
      m_PrevImgs.pop_back();
      return true;
    } else {
      m_PrevImgs.pop_back();
      return SetPrevious();
    }
  }
  return false;
}

bool Wallpaper::UndoDelete() {
  if (m_BlackList.empty())
    return false;
  const auto& back = m_BlackList.back();
  if (!back.second.has_parent_path()) {
    m_BlackList.pop_back();
    WriteBlackList();
    return false;
  }
  auto parent_path = back.second.parent_path();
  if (!fs::exists(parent_path)) {
    fs::create_directories(parent_path);
  }
  fs::rename(back.first, back.second);
  if (!SetWallpaper(back.second))
    return false;
  m_PrevImgs.push_back(m_CurImage);
  m_CurImage = std::move(back.second);
  m_CurImage.make_preferred();
  m_BlackList.pop_back();
  m_Wallpaper->UndoDislike(m_CurImage.u8string());
  WriteBlackList();
  return true;
}

bool Wallpaper::ClearJunk() {
  constexpr char8_t junk[] = u8"junk";
  if (fs::exists(junk))
    fs::remove_all(junk);
  if (fs::exists("Blacklist.txt"))
    fs::remove("Blacklist.txt");
  m_BlackList.clear();
  return false;
}

bool Wallpaper::SetFavorite() {
  m_Favorites->UndoDislike(m_CurImage.u8string());
  return true;
}

bool Wallpaper::UnSetFavorite() {
  m_Favorites->Dislike(m_CurImage.u8string());
  return true;
}

const std::wstring Wallpaper::m_ImgNamePattern = L".*\\.(jpg|bmp|gif|jpeg|png)$";
bool Wallpaper::IsImageFile(const fs::path& filesName) {
  // BMP, PNG, GIF, JPG
  std::wregex pattern(m_ImgNamePattern, std::regex::icase);
  return std::regex_match(filesName.wstring(), pattern);
}

bool Wallpaper::SetDropFile(std::vector<std::u8string> urls) {
  if (WallBase::ms_IsWorking) {
    glb->glbShowMsgbox(u8"提示", u8"目前没空！");
    return false;
  }
  WallBase::ms_IsWorking = true;
  auto lst = urls | std::views::filter([](const std::u8string& i) {
    auto iter = std::find(i.crbegin(), i.crend(), u8'.');
    if (iter == i.crend()) return false;
    const std::wregex pattern(m_ImgNamePattern, std::wregex::icase);
    return std::regex_match(std::wstring(--iter.base(), i.cend()), pattern);
  });
  m_NextImgsBuffer.insert(m_NextImgsBuffer.end(), lst.begin(), lst.end());
  if (!lst.empty()) {
    std::thread([this]() {
      SetNext();
      WriteSettings();
      WallBase::ms_IsWorking = false;
    }).detach();
    return true;
  } else {
    WallBase::ms_IsWorking = false;
    return false;
  }
}

bool Wallpaper::RemoveCurrent() {
  if (m_NextImgs.empty()) {
    if (!SetNext())
      return false;
    if (fs::exists(m_PrevImgs.back()))
      AppendBlackList(m_PrevImgs.back());
    m_Wallpaper->Dislike(m_PrevImgs.back().u8string());
    m_PrevImgs.pop_back();
    return true;
  } else {
    if (SetWallpaper(m_NextImgs.top())) {
      if (fs::exists(m_CurImage))
        AppendBlackList(m_CurImage);
      m_Wallpaper->Dislike(m_CurImage.u8string());
      m_CurImage = m_NextImgs.top();
      m_CurImage.make_preferred();
      m_NextImgs.pop();
      return true;
    } else {
      m_NextImgs.pop();
      return RemoveCurrent();
    }
  }
  return false;
}

void Wallpaper::ReadSettings() {
  std::ifstream file("History.txt", std::ios::in);
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
  file.open("Blacklist.txt", std::ios::in);
  if (file.is_open()) {
    std::string key, val;
    while (std::getline(file, key) && std::getline(file, val)) {
      m_BlackList.emplace_back(key, val);
    }
    file.close();
  }
}

void Wallpaper::WriteSettings() const {
  int m_CountLimit = 100;
  std::ofstream file("History.txt", std::ios::out);
  if (!file.is_open())
    return;
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
  auto& back = m_BlackList.emplace_back(junk / path.filename(), path);
  fs::rename(back.second, back.first);

  std::ofstream file("Blacklist.txt", std::ios::app);
  if (!file.is_open())
    return;
  file << back.first.string() << std::endl << back.second.string() << std::endl;
}

void Wallpaper::WriteBlackList() const {
  std::ofstream file("Blacklist.txt", std::ios::out);
  if (!file.is_open())
    return;
  for (auto& i : m_BlackList) {
    file << i.first.string() << std::endl << i.second.string() << std::endl;
  }
  file.close();
}

void Wallpaper::SetAutoChange(bool flag) {
  m_Timer->Expire();
  auto& jsAutoChange = m_Settings[u8"AutoChange"];
  if (jsAutoChange.isTrue() != flag) {
    jsAutoChange.setValue(flag);
    SettingsCallback();
  }
  if (flag) {
    m_Timer->StartTimer(GetTimeInterval(),
                        std::bind(&Wallpaper::SetSlot, this, 1));
  }
}

void Wallpaper::SetFirstChange(bool flag) {
  auto& jsFirstChange = m_Settings[u8"FirstChange"];
  if (jsFirstChange.isTrue() != flag) {
    jsFirstChange.setValue(flag);
    SettingsCallback();
  }
  if (flag) {
    SetSlot(1);
  }
}

void Wallpaper::SetCurDir(fs::path str) {
  str.make_preferred();
  if (fs::exists(str) || fs::create_directory(str))
    m_Wallpaper->SetCurDir(str.u8string());
}

bool Wallpaper::SetImageType(int index) {
  if (WallBase::ms_IsWorking) {
    return false;
  }
  auto& jsImageType = m_Settings[u8"ImageType"];
  if (jsImageType.getValueInt() != index) {
    jsImageType.setValue(index);
    SettingsCallback();
  }

  m_Wallpaper = WallBase::GetNewInstance(*m_Config, index);
  return true;
}

bool Wallpaper::IsWorking() {
  return WallBase::ms_IsWorking;
}

// attention: thread maybe working!
