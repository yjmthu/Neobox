#include <unordered_set>
#ifdef _WIN32
#include <Shlobj.h>
#endif  // _WIN32

#include "bingapi.h"
#include "directapi.h"
#include "favorite.h"
#include "native.h"
#include "scriptoutput.h"
#include "wallhaven.h"

std::filesystem::path GetSpecialFolderPath(int type) {
  std::wstring result(MAX_PATH, 0);
  SHGetSpecialFolderPathW(nullptr, result.data(), type, TRUE);
  result.erase(result.find(L'\0'));
  return result;
}

static WallBase* pFavorite = nullptr;
static WallBase* pBingApi = nullptr;
static WallBase* pOther = nullptr;

std::unordered_set<std::filesystem::path> m_UsingFiles;

std::atomic_bool WallBase::m_IsWorking = false;

const std::filesystem::path WallBase::m_HomePicLocation =
    GetSpecialFolderPath(CSIDL_MYPICTURES) / L"桌面壁纸";

WallBase* WallBase::GetNewInstance(int type) {
  if (pOther) {
    delete pOther;
    pOther = nullptr;
  }
  switch (type) {
    case WALLHAVEN:
      return pOther = new Wallhaven;
    case BINGAPI:
      if (!pBingApi)
        pBingApi = new BingApi;
      return pBingApi;
    case DIRECTAPI:
      return pOther = new DirectApi;
    case NATIVE:
      return pOther = new Native;
    case SCRIPTOUTPUT:
      return pOther = new ScriptOutput;
    case FAVORITE:
      if (!pFavorite)
        pFavorite = new Favorite;
      return pFavorite;
    default:
      return pOther;
  }
}

void WallBase::ClearInstatnce() {
  delete pOther;
  delete pFavorite;
  delete pBingApi;
}

void WallBase::Dislike(const std::u8string& sImgPath) {}

void WallBase::UndoDislike(const std::u8string& sImgPath) {}

void WallBase::SetCurDir(const std::u8string& sImgDir) {}

