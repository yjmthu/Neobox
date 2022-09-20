#include <unordered_set>

#include "bingapi.h"
#include "directapi.h"
#include "favorite.h"
#include "native.h"
#include "scriptoutput.h"
#include "wallhaven.h"

std::unordered_set<std::filesystem::path> m_UsingFiles;

std::atomic_bool WallBase::m_IsWorking = false;

WallBase* WallBase::GetNewInstance(const std::filesystem::path& picHome,
                                   int type) {
  static WallBase* pFavorite = new Favorite(picHome);
  static WallBase* pBingApi = new BingApi(picHome);
  switch (type) {
    case WALLHAVEN:
      return new Wallhaven(picHome);
    case BINGAPI:
      return pBingApi;
    case DIRECTAPI:
      return new DirectApi(picHome);
    case NATIVE:
      return new Native(picHome);
    case SCRIPTOUTPUT:
      return new ScriptOutput(picHome);
    case FAVORITE:
      return pFavorite;
    default:
      return nullptr;
  }
}

void WallBase::Dislike(const std::u8string& sImgPath) {}

void WallBase::UndoDislike(const std::u8string& sImgPath) {}

void WallBase::SetCurDir(const std::u8string& sImgDir) {}
