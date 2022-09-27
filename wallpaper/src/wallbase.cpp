#include <wallbase.h>
#include <unordered_set>
#ifdef _WIN32
#include <Shlobj.h>
#endif  // _WIN32

#if 0
import wallpaper1;
import wallpaper2;
import wallpaper3;
import wallpaper4;
import wallpaper5;
import wallpaper6;
#else
#include <mo_bingapi.h>
#include <mo_directapi.h>
#include <mo_favorie.h>
#include <mo_native.h>
#include <mo_scriptoutput.h>
#include <mo_wallhaven.h>
#endif

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

const fs::path WallBase::ms_HomePicLocation =
    GetSpecialFolderPath(CSIDL_MYPICTURES) / L"桌面壁纸";

WallBase* WallBase::GetNewInstance(int type) {
  if (s_pOther) {
    delete s_pOther;
    s_pOther = nullptr;
  }
  switch (type) {
    case WALLHAVEN:
      return s_pOther = new Wallhaven;
    case BINGAPI:
      if (!s_pBingApi)
        s_pBingApi = new BingApi;
      return s_pBingApi;
    case DIRECTAPI:
      return s_pOther = new DirectApi;
    case NATIVE:
      return s_pOther = new Native;
    case SCRIPTOUTPUT:
      return s_pOther = new ScriptOutput;
    case FAVORITE:
      if (!s_pFavorite)
        s_pFavorite = new Favorite;
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
