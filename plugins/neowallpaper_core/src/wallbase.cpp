#include <wallbase.h>
#include <unordered_set>
#ifdef _WIN32
#include <Shlobj.h>
#endif  // _WIN32

#include <bingapi.h>
#include <directapi.h>
#include <favorie.h>
#include <native.h>
#include <scriptoutput.h>
#include <wallhaven.h>

fs::path GetSpecialFolderPath() {
#ifdef _WIN32
  std::wstring result(MAX_PATH, 0);
  SHGetSpecialFolderPathW(nullptr, result.data(), CSIDL_MYPICTURES, TRUE);
  result.erase(result.find(L'\0'));
#else
  fs::path result = std::getenv("HOME");
  result /= "Pictures";
  if (!fs::exists(result)) {
    fs::create_directory(result);
  }
#endif
  return result;
}

std::mutex WallBase::m_DataMutex;
const fs::path WallBase::m_DataDir = u8"wallpaperData";
const fs::path WallBase::m_ConfigPath = u8"wallpaperData/Wallpaper.json";
std::atomic_bool WallBase::m_QuitFlag = false;
std::function<void()> WallBase::SaveSetting;


std::array<WallBase*, WallBase::NONE> WallBase::m_Instances {};

std::nullptr_t WallBase::Initialize(YJson& setting) {
  SaveSetting = std::bind(&YJson::toFile, &setting, m_ConfigPath, true, YJson::UTF8);

  m_Instances = {
    new Wallhaven(setting[u8"壁纸天堂"]),
    new BingApi(setting[u8"必应壁纸"]),
    new DirectApi(setting[u8"直链壁纸"]),
    new Native(setting[u8"本地壁纸"]),
    new ScriptOutput(setting[u8"脚本输出"]),
    new Favorite(setting[u8"收藏壁纸"]),
  };

  return nullptr;
}

WallBase* WallBase::GetInstance(uint32_t type) {
  if (type >= m_Instances.size()) type = 0;
  return m_Instances[type];
}

void WallBase::Uuinitialize() {
  for (auto& ptr: m_Instances) {
    delete ptr;
    ptr = nullptr;
  }
}

void WallBase::Dislike(std::u8string_view sImgPath) {}

void WallBase::UndoDislike(std::u8string_view sImgPath) {}

void WallBase::SetJson(const YJson& json) {
  m_DataMutex.lock();
  m_Setting = json;
  SaveSetting();
  m_DataMutex.unlock();
}

YJson WallBase::GetJson() const {
  Locker locker(m_DataMutex);
  return m_Setting;
}

fs::path WallBase::GetHomePicLocation() {
  return GetSpecialFolderPath() / u8"桌面壁纸";
}

std::u8string WallBase::GetStantardDir(const std::u8string& name)
{
  auto initDir = GetHomePicLocation() / name;
  initDir.make_preferred();
  return initDir.u8string();
}
