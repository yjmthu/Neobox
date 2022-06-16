#include "speedbox.h"

#include <netspeedhelper.h>

#include <QQuickView>
#include <KWindowEffects>

#include "speedapp.h"
#include "speedmenu.h"

SpeedBox::SpeedBox()
    : QObject(nullptr)
    , m_NetSpeedHelper(new NetSpeedHelper)
{
}

SpeedBox::~SpeedBox() {
  delete m_NetSpeedHelper;
}

void SpeedBox::updateInfo() {
  const char* li[] = {"MemUseage", "NetUpSpeed", "NetDownSpeed"};
  m_NetSpeedHelper->GetSysInfo();
  QObject *label = nullptr;
  for (size_t i=0; i<3; ++i) {
    label = m_VarBox->m_SpeedBox->findChild<QObject*>(li[i]);
    label->setProperty("text", QString::fromUtf8(
      reinterpret_cast<const char*>(m_NetSpeedHelper->m_SysInfo[i].data()),
      static_cast<int>(m_NetSpeedHelper->m_SysInfo[i].size())));
  }
}

void SpeedBox::showMenu(int x, int y)
{
  m_VarBox->m_Menu->popup(QPoint(x, y));
}

void SpeedBox::setRoundRect(int x, int y, int w, int h, int r, bool set)
{
  if (!set) {
    KWindowEffects::enableBlurBehind(m_VarBox->m_SpeedBox, false);
    return;
  }
  QRegion region;
  QRect rect(x, y, w, h);
  region += rect.adjusted(r, 0, -r, 0);
  region += rect.adjusted(0, r, 0, -r);

  // top left
  QRect corner(rect.topLeft(), QSize(r*2, r*2));
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
  
  KWindowEffects::enableBlurBehind(m_VarBox->m_SpeedBox, true, region);
}
