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

fs::path GetSpecialFolderPath(int type) {
  std::wstring result(MAX_PATH, 0);
  SHGetSpecialFolderPathW(nullptr, result.data(), type, TRUE);
  result.erase(result.find(L'\0'));
  return result;
}

static WallBase* s_pFavorite = nullptr;
static WallBase* s_pBingApi = nullptr;
static WallBase* s_pOther = nullptr;

std::unordered_set<fs::path> g_UsingFiles;
std::atomic_bool WallBase::ms_IsWorking = false;
const fs::path WallBase::m_DataDir = u8"wallpaperData";
const fs::path WallBase::m_ConfigPath = u8"wallpaperData/Wallpaper.json";
std::function<void()> WallBase::SaveSetting = [](){};

const fs::path WallBase::ms_HomePicLocation =
    GetSpecialFolderPath(CSIDL_MYPICTURES) / L"桌面壁纸";

WallBase* WallBase::GetNewInstance(YJson& setting, int type) {
  if (s_pOther) {
    delete s_pOther;
    s_pOther = nullptr;
  }
  SaveSetting = std::bind(&YJson::toFile, &setting, m_ConfigPath, true, YJson::UTF8);
  switch (type) {
    case WALLHAVEN:
      return s_pOther = new Wallhaven(setting[u8"壁纸天堂"]);
    case BINGAPI:
      if (!s_pBingApi)
        s_pBingApi = new BingApi(setting[u8"必应壁纸"]);
      return s_pBingApi;
    case DIRECTAPI:
      return s_pOther = new DirectApi(setting[u8"直链壁纸"]);
    case NATIVE:
      return s_pOther = new Native(setting[u8"本地壁纸"]);
    case SCRIPTOUTPUT:
      return s_pOther = new ScriptOutput(setting[u8"脚本输出"]);
    case FAVORITE:
      if (!s_pFavorite)
        s_pFavorite = new Favorite(setting[u8"收藏壁纸"]);
      return s_pFavorite;
    default:
      return s_pOther;
  }
}

void WallBase::ClearInstatnce() {
  delete s_pOther;
  delete s_pFavorite;
  delete s_pBingApi;
}

void WallBase::Dislike(const std::u8string& sImgPath) {}

void WallBase::UndoDislike(const std::u8string& sImgPath) {}

void WallBase::SetCurDir(const std::u8string& sImgDir) {}

std::u8string WallBase::GetStantardDir(const std::u8string& name)
{
  auto initDir = ms_HomePicLocation / name;
  initDir.make_preferred();
  return initDir.u8string();
}