#include <unordered_set>

#include "bingapi.hpp"
#include "directapi.hpp"
#include "native.hpp"
#include "scriptoutput.hpp"
#include "wallhaven.hpp"

std::unordered_set<std::filesystem::path> m_UsingFiles;

std::atomic_bool WallBase::m_IsWorking = false;

WallBase *WallBase::GetNewInstance(const std::filesystem::path &picHome, int type)
{
    switch (type)
    {
    case 0:
        return new WallClass::Wallhaven(picHome);
    case 1:
        return new WallClass::BingApi(picHome);
    case 2:
        return new WallClass::DirectApi(picHome);
    case 3:
        return new WallClass::Native(picHome);
    case 4:
        return new WallClass::ScriptOutput(picHome);
    default:
        return nullptr;
    }
}
