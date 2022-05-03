#include "wallhaven.hpp"
#include "bingapi.hpp"
#include "scriptoutput.hpp"
#include "native.hpp"
#include "directapi.hpp"

bool WallBase::m_IsWorking = false;
// int WallBase::m_JobLeft = 0;

WallBase* WallBase::GetNewInstance(int type) {
    switch (type) {
    case 0:
        return new WallClass::Wallhaven;
    case 1:
        return new WallClass::BingApi;
    case 2:
        return new WallClass::DirectApi;
    case 3:
        return new WallClass::Native;
    case 4:
        return new WallClass::ScriptOutput;
    default:
        return nullptr;
    }
}
