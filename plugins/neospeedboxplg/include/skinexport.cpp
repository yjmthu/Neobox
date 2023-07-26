#ifdef Skin

#ifdef SPEEDBOX_SKIN_STATIC

SkinObject * newSkin(QWidget* parent, const TrafficInfo& trafficinfo)
{
  return new Skin(parent, trafficinfo);
}

#else

#include <QWidget>
#include <string>

#include <skinobject.h>
#include <trafficinfo.h>

#ifdef _WIN32
extern "C" _declspec(dllexport)
#else
extern "C"
#endif
bool skinVersion(const std::string& date)
{
  return date == __DATE__;
}

#ifdef _WIN32
extern "C" _declspec(dllexport)
#else
extern "C"
#endif
SkinObject * newSkin(QWidget* parent, const TrafficInfo& trafficinfo)
{
  return new Skin(parent, trafficinfo);
}

#endif // SPEEDBOX_SKIN_STATIC

#endif // Skin
