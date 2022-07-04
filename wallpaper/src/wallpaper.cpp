#include <httplib.h>
#include <sysapi.h>
#include <timer.h>

#include <apiclass.hpp>
#include <ranges>
#include <regex>
#include <unordered_set>

#ifdef _WIN32
#include <wininet.h>
#endif  // _WIN32

extern std::unordered_set<std::filesystem::path> m_UsingFiles;

extern void ShowMessage(const std::u8string& title, const std::u8string& text,
                        int type = 0);

constexpr char Wallpaper::m_szWallScript[];

bool Wallpaper::DownloadImage(const ImageInfoEx& imageInfo) {
  if (imageInfo->empty()) return false;
  auto dir = imageInfo->front().substr(
      0, imageInfo->front().find_last_of(
             std::filesystem::path::preferred_separator));
  if (!std::filesystem::exists(dir)) std::filesystem::create_directories(dir);
  if (std::filesystem::exists(imageInfo->front())) {
    if (!std::filesystem::file_size(imageInfo->front()))
      std::filesystem::remove(imageInfo->front());
    else
      return true;
  }
  if (imageInfo->size() != 3) return false;
  try {
    httplib::Client clt(
        std::string(imageInfo->at(1).begin(), imageInfo->at(1).end()));
    std::ofstream file(std::filesystem::path(imageInfo->front()),
                       std::ios::binary | std::ios::out);
    if (!file.is_open()) return false;
    m_UsingFiles.emplace(imageInfo->front());
    auto m_fHandleData = [&file](const char* data, size_t length) {
      file.write(data, length);
      return true;
    };
    auto res = clt.Get(reinterpret_cast<const char*>(imageInfo->at(2).c_str()),
                       m_fHandleData);
  label:
    if (res && res->status == 200) {
      m_UsingFiles.erase(imageInfo->front());
      file.close();
      return true;
    } else if (res && (res->status == 301 || res->status == 302)) {
      file.seekp(std::ios::beg);
      clt.set_follow_location(true);
      res = clt.Get(reinterpret_cast<const char*>(imageInfo->at(2).c_str()),
                    m_fHandleData);
      goto label;
    } else {
      m_UsingFiles.erase(imageInfo->front());
      file.close();
      if (std::filesystem::exists(imageInfo->front()))
        std::filesystem::remove(imageInfo->front());
      return false;
    }
  } catch (...) {
    ShowMessage(u8"网络错误", u8"从：" + imageInfo->at(1) + imageInfo->at(2) +
                                  u8"\n下载文件：" + imageInfo->front() +
                                  u8"\n发生错误。");
    return false;
  }
}

Wallpaper::Desktop Wallpaper::GetDesktop() {
#if defined(_WIN32)
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

bool Wallpaper::SetWallpaper(const std::filesystem::path& imagePath) {
  [[maybe_unused]] static auto m_DesktopType = GetDesktop();
  if (!std::filesystem::exists(imagePath)) return false;
#if defined(_WIN32)
#ifdef UNICODE
  std::wstring str = imagePath.wstring();
#else
  std::string str = imagePath.string();
#endif  //  UNICDOE
  return ::SystemParametersInfo(SPI_SETDESKWALLPAPER, UINT(0),
                                const_cast<TCHAR*>(str.c_str()),
                                SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
#elif defined(__linux__)
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
      std::cout << "不支持的桌面类型；\n";
      return false;
  }
  sstr << imagePath;
  std::string m_sCmd = sstr.str();
  std::cout << imagePath << std::endl << m_sCmd << std::endl;
  return system(m_sCmd.c_str()) == 0;
#endif
}

#if 0
bool Wallpaper::IsOnline() {
#ifdef _WIN32
  DWORD flags;
  return InternetGetConnectedState(&flags, 0);
#elif 0
  std::vector<std::string> result;
  GetCmdOutput<char>("ping www.baidu.com -c 2", result);
  if (result.size() < 2) return false;
  auto& data = result.end()[-2];
  auto first = data.find("received");
  if (first == std::string::npos) return false;
  first += 10;
  auto last = data.find("%", first);
  auto&& lostPacket = data.substr(first, last - first);
  std::cout << data << std::endl << "lostPacket: " << lostPacket << std::endl;
  return !std::atoi(lostPacket.c_str());
#else
  httplib::Client clt("https://www.baidu.com");
  auto res = clt.Get("/");
  return res && res->status == 200;
#endif
}
#endif

Wallpaper::Wallpaper(const std::filesystem::path& picHome)
    : m_ImageType(-1),
      m_TimeInterval(15),
      m_AutoChange(false),
      m_FirstChange(false),
      m_Wallpaper(nullptr),
      m_PicHomeDir(picHome),
      m_Timer(new Timer) {
  ReadSettings();
}

Wallpaper::~Wallpaper() {
  delete m_Timer;
  delete m_Wallpaper;
  while (!m_Jobs.empty()) {
    delete m_Jobs.front();
    m_Jobs.pop();
  }
  for (const auto& i :
       m_UsingFiles |
           std::views::filter((bool (*)(const std::filesystem::path&)) &
                              std::filesystem::exists)) {
    std::filesystem::remove(i);
  }
}

void Wallpaper::SetSlot(int type) {
  using namespace std::literals;
  if (WallBase::m_IsWorking) {
    ShowMessage(u8"提示", u8"后台正忙，请稍后！");
    return;
  }
  WallBase::m_IsWorking = true;
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
    WallBase::m_IsWorking = false;
    if (m_AutoChange) {
      m_Timer->Expire();
      m_Timer->StartTimer(m_TimeInterval,
                          std::bind(&Wallpaper::SetSlot, this, 1));
    }
  }).detach();
}

const std::filesystem::path& Wallpaper::GetImageDir() const {
  return m_Wallpaper->GetImageDir();
}

void Wallpaper::SetTimeInterval(int minute) {
  m_TimeInterval = minute;
  m_Timer->Expire();
  if (!m_AutoChange) return;
  m_Timer->Expire();
  m_Timer->StartTimer(m_TimeInterval, std::bind(&Wallpaper::SetSlot, this, 1));
}

bool Wallpaper::SetNext() {
  if (m_NextImgs.empty()) {
    const ImageInfoEx ptr = m_Wallpaper->GetNext();
    if (ptr->empty() || !DownloadImage(ptr)) return false;
    if (!SetWallpaper(ptr->front())) return false;
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

bool Wallpaper::SetPrevious() {
  if (!m_PrevImgs.empty()) {
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

bool Wallpaper::UndoDelete() {
  if (m_BlackList.empty()) return false;
  auto& back = m_BlackList.back();
  if (!back.second.has_parent_path()) {
    m_BlackList.pop_back();
    WriteBlackList();
    return false;
  }
  auto parent_path = back.second.parent_path();
  if (!std::filesystem::exists(parent_path)) {
    std::filesystem::create_directories(parent_path);
  }
  std::filesystem::rename(back.first, back.second);
  m_PrevImgs.push_back(m_CurImage);
  SetWallpaper(m_CurImage = std::move(back.second));
  m_BlackList.pop_back();
  WriteBlackList();
  return true;
}

bool Wallpaper::ClearJunk() {
  constexpr char8_t junk[] = u8"junk";
  if (std::filesystem::exists(junk)) std::filesystem::remove_all(junk);
  if (std::filesystem::exists("Blacklist.txt"))
    std::filesystem::remove("Blacklist.txt");
  m_BlackList.clear();
  return false;
}

bool Wallpaper::IsImageFile(const std::filesystem::path& filesName) {
  // BMP, PNG, GIF, JPG
  std::regex pattern(".*\\.(jpg|bmp|gif|jpeg|png)$", std::regex::icase);
  return std::regex_match(filesName.string(), pattern);
}

bool Wallpaper::SetDropFile(std::deque<std::filesystem::path>&& paths) {
  if (WallBase::m_IsWorking) return false;
  WallBase::m_IsWorking = true;
  for (auto& i: paths)
    m_NextImgs.push(std::move(i));
  SetNext();
  WallBase::m_IsWorking = false;
  return true;
}

bool Wallpaper::RemoveCurrent() {
  if (m_NextImgs.empty()) {
    if (!SetNext()) return false;
    if (std::filesystem::exists(m_PrevImgs.back()))
      AppendBlackList(m_PrevImgs.back());
    m_Wallpaper->Dislike(m_PrevImgs.back());
    m_PrevImgs.pop_back();
    return true;
  } else {
    if (SetWallpaper(m_NextImgs.top())) {
      if (std::filesystem::exists(m_CurImage)) AppendBlackList(m_CurImage);
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

void Wallpaper::ReadSettings() {
  std::ifstream file("History.txt", std::ios::in);
  if (file.is_open()) {
    std::string temp;
    if (std::getline(file, temp)) {
      m_CurImage = temp;
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
  if (!file.is_open()) return;
  if (!m_CurImage.empty()) file << m_CurImage.string() << std::endl;
  for (auto i = m_PrevImgs.rbegin(); i != m_PrevImgs.rend(); ++i) {
    file << i->string() << std::endl;
    if (!--m_CountLimit) break;
  }
  file.close();
}

void Wallpaper::AppendBlackList(const std::filesystem::path& path) {
  constexpr char8_t junk[] = u8"junk";
  if (!std::filesystem::exists(junk)) {
    std::filesystem::create_directory(junk);
  }
  auto& back = m_BlackList.emplace_back(junk / path.filename(), path);
  std::filesystem::rename(back.second, back.first);

  std::ofstream file("Blacklist.txt", std::ios::app);
  if (!file.is_open()) return;
  file << back.first.string() << std::endl << back.second.string() << std::endl;
}

void Wallpaper::WriteBlackList() const {
  std::ofstream file("Blacklist.txt", std::ios::out);
  if (!file.is_open()) return;
  for (auto& i : m_BlackList) {
    file << i.first.string() << std::endl << i.second.string() << std::endl;
  }
  file.close();
}

void Wallpaper::SetAutoChange(bool flag) {
  m_Timer->Expire();
  if ((m_AutoChange = flag)) {
    m_Timer->StartTimer(m_TimeInterval,
                        std::bind(&Wallpaper::SetSlot, this, 1));
  }
}

void Wallpaper::SetFirstChange(bool flag) {
  if ((m_FirstChange = flag)) {
    SetSlot(1);
  } else {
  }
}

void Wallpaper::SetCurDir(const std::filesystem::path& str) {
  if (!std::filesystem::exists(str)) std::filesystem::create_directory(str);
  m_Wallpaper->SetCurDir(str);
}

bool Wallpaper::SetImageType(int index) {
  m_ImageType = index;
  if (WallBase::m_IsWorking) {
    m_Jobs.push(m_Wallpaper);
    m_Wallpaper = nullptr;
  } else {
    delete m_Wallpaper;
  }
  m_Wallpaper = WallBase::GetNewInstance(m_PicHomeDir, index);
  if (index == 1) return true;
  std::thread([this]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    WallBase* ptr = WallBase::GetNewInstance(m_PicHomeDir, 1);
    for (int i = 0; i < 7; ++i) {
      Wallpaper::DownloadImage(ptr->GetNext());
    }
    delete ptr;
  }).detach();
  return true;
}

bool Wallpaper::IsWorking() { return WallBase::m_IsWorking; }

// attention: thread maybe working!

std::u8string Wallpaper::GetJson() const { return m_Wallpaper->GetJson(); }

void Wallpaper::SetJson(const std::u8string& str) { m_Wallpaper->SetJson(str); }
