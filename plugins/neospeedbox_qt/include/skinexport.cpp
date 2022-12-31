#ifdef Skin

#include <QWidget>
#include <string>

#include <skinobject.h>
#include <trafficinfo.h>

extern "C" _declspec(dllexport) bool skinVersion(const std::string& date)
{
  return date == __DATE__;
}

extern "C" _declspec(dllexport) SkinObject * newSkin(QWidget* parent, const TrafficInfo& trafficinfo)
{
  return new Skin(parent, trafficinfo);
}

#endif
