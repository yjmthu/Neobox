#include "speedbox.h"

#include <netspeedhelper.h>

#include <QQuickView>
#ifdef __linux__
#include <KWindowEffects>
#endif

#include "speedapp.h"
#include "speedmenu.h"

SpeedBox::SpeedBox(QObject* parent) : QObject(parent), m_NetSpeedHelper(new NetSpeedHelper) {}

SpeedBox::~SpeedBox() { delete m_NetSpeedHelper; }

void SpeedBox::updateInfo() {
  m_NetSpeedHelper->GetSysInfo();
  emit memUseageChanged();
  emit netUpSpeedChanged();
  emit netDownSpeedChanged();
}

int SpeedBox::memUseage() const {
  return std::get<0>(m_NetSpeedHelper->m_SysInfo);
}

double SpeedBox::netUpSpeed() const {
  return static_cast<double>(std::get<1>(m_NetSpeedHelper->m_SysInfo));
}

double SpeedBox::netDownSpeed() const {
  return static_cast<double>(std::get<2>(m_NetSpeedHelper->m_SysInfo));
}

void SpeedBox::setRoundRect(int x, int y, int w, int h, int r, bool set) {
#ifdef __linux__
  if (!set) {
    KWindowEffects::enableBlurBehind(qobject_cast<QWindow*>(parent()->parent()), false);
    return;
  }
  QRegion region;
  QRect rect(x, y, w, h);
  region += rect.adjusted(r, 0, -r, 0);
  region += rect.adjusted(0, r, 0, -r);

  // top left
  QRect corner(rect.topLeft(), QSize(r * 2, r * 2));
  region += QRegion(corner, QRegion::Ellipse);

  // top right
  corner.moveTopRight(rect.topRight());
  region += QRegion(corner, QRegion::Ellipse);

  // bottom left
  corner.moveBottomLeft(rect.bottomLeft());
  region += QRegion(corner, QRegion::Ellipse);

  // bottom right
  corner.moveBottomRight(rect.bottomRight());
  region += QRegion(corner, QRegion::Ellipse);

  KWindowEffects::enableBlurBehind(qobject_cast<QWindow*>(parent()->parent()), true, region);
#endif
}
