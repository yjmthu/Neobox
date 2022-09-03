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
  static WallBase* pFavorite = new WallClass::Favorite(picHome);
  switch (type) {
    case WallClass::WALLHAVEN:
      return new WallClass::Wallhaven(picHome);
    case WallClass::BINGAPI:
      return new WallClass::BingApi(picHome);
    case WallClass::DIRECTAPI:
      return new WallClass::DirectApi(picHome);
    case WallClass::NATIVE:
      return new WallClass::Native(picHome);
    case WallClass::SCRIPTOUTPUT:
      return new WallClass::ScriptOutput(picHome);
    case WallClass::FAVORITE:
      return pFavorite;
    default:
      return nullptr;
  }
}

void WallBase::Dislike(const std::u8string& sImgPath) {}

void WallBase::UndoDislike(const std::u8string& sImgPath) {}

void WallBase::SetCurDir(const std::u8string& sImgDir) {}
