#include "speedbox.h"

#include <netspeedhelper.h>
#include <translater.h>
#include <wallpaper.h>
#include <yjson.h>

#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>
#include <appcode.hpp>

#include "speedapp.h"
#include "speedmenu.h"

extern std::unique_ptr<YJson> m_GlobalSetting;
extern const char* m_szClobalSettingFile;

void SpeedBox::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    m_LastPos = event->pos();
    setMouseTracking(true);
  } else if (event->button() == Qt::RightButton) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    m_VarBox->m_Menu->popup(event->globalPos());
#else
    m_Menu->popup(event->globalPosition().toPoint());
#endif
  } else if (event->button() == Qt::MiddleButton) {
    qApp->exit(static_cast<int>(ExitCode::RETCODE_RESTART));
  } else {
    // TO DO
  }
}

void SpeedBox::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    setMouseTracking(false);
    WritePosition();
  }
}

void SpeedBox::mouseMoveEvent(QMouseEvent* event) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  move(event->globalPos() - m_LastPos);
#else
  move(event->globalPosition().toPoint() - m_LastPos);
#endif
}

void SpeedBox::mouseDoubleClickEvent(QMouseEvent* event) {
  m_VarBox->m_Translater->IntelligentShow();
  event->accept();
}

void SpeedBox::keyPressEvent(QKeyEvent* event) {
  if (!m_ChangeMode) {
    event->accept();
    return;
  }
  switch (event->key()) {
    case Qt::Key::Key_Left:
      --std::get<2>(m_Style[m_ChangeMode - 1]).first;
      break;
    case Qt::Key::Key_Right:
      ++std::get<2>(m_Style[m_ChangeMode - 1]).first;
      break;
    case Qt::Key::Key_Up:
      --std::get<2>(m_Style[m_ChangeMode - 1]).second;
      break;
    case Qt::Key::Key_Down:
      ++std::get<2>(m_Style[m_ChangeMode - 1]).second;
      break;
    default:
      event->accept();
      return;
  }
  update();
  event->accept();
}

void SpeedBox::paintEvent(QPaintEvent*) {
  QPainter painter;
  painter.begin(this);
  painter.setBrush(QBrush(m_BackCol));
  painter.setPen(Qt::transparent);
  painter.drawRoundedRect(QRect(0, 0, width(), height()), 3, 3);  // round rect
  painter.setFont(std::get<1>(m_Style[0]));
  painter.setPen(std::get<0>(m_Style[0]));
  switch (m_Side) {
    case SideType::Left:
      painter.drawText(
          width() + std::get<2>(m_Style[0]).first - m_Width,
          std::get<2>(m_Style[0]).second,
          QString::fromUtf8(reinterpret_cast<const char*>(
                                m_NetSpeedHelper->m_SysInfo[0].data()),
                            m_NetSpeedHelper->m_SysInfo[0].size()));
      painter.setPen(std::get<0>(m_Style[1]));
      painter.setFont(std::get<1>(m_Style[1]));
      painter.drawText(
          width() + std::get<2>(m_Style[1]).first - m_Width,
          std::get<2>(m_Style[1]).second,
          QString::fromUtf8(reinterpret_cast<const char*>(
                                m_NetSpeedHelper->m_SysInfo[1].data()),
                            m_NetSpeedHelper->m_SysInfo[1].size()));
      painter.setPen(std::get<0>(m_Style[2]));
      painter.setFont(std::get<1>(m_Style[2]));
      painter.drawText(
          width() + std::get<2>(m_Style[2]).first - m_Width,
          std::get<2>(m_Style[2]).second,
          QString::fromUtf8(reinterpret_cast<const char*>(
                                m_NetSpeedHelper->m_SysInfo[2].data()),
                            m_NetSpeedHelper->m_SysInfo[2].size()));
      break;
    case SideType::Top:
      painter.drawText(
          std::get<2>(m_Style[0]).first,
          height() + std::get<2>(m_Style[0]).second - m_Height,
          QString::fromUtf8(reinterpret_cast<const char*>(
                                m_NetSpeedHelper->m_SysInfo[0].data()),
                            m_NetSpeedHelper->m_SysInfo[0].size()));
      painter.setPen(std::get<0>(m_Style[1]));
      painter.setFont(std::get<1>(m_Style[1]));
      painter.drawText(
          std::get<2>(m_Style[1]).first,
          height() + std::get<2>(m_Style[1]).second - m_Height,
          QString::fromUtf8(reinterpret_cast<const char*>(
                                m_NetSpeedHelper->m_SysInfo[1].data()),
                            m_NetSpeedHelper->m_SysInfo[1].size()));
      painter.setPen(std::get<0>(m_Style[2]));
      painter.setFont(std::get<1>(m_Style[2]));
      painter.drawText(
          std::get<2>(m_Style[2]).first,
          height() + std::get<2>(m_Style[2]).second - m_Height,
          QString::fromUtf8(reinterpret_cast<const char*>(
                                m_NetSpeedHelper->m_SysInfo[2].data()),
                            m_NetSpeedHelper->m_SysInfo[2].size()));
      break;
    case SideType::Right:
    case SideType::Bottom:
      painter.drawText(
          std::get<2>(m_Style[0]).first, std::get<2>(m_Style[0]).second,
          QString::fromUtf8(reinterpret_cast<const char*>(
                                m_NetSpeedHelper->m_SysInfo[0].data()),
                            m_NetSpeedHelper->m_SysInfo[0].size()));
      painter.setPen(std::get<0>(m_Style[1]));
      painter.setFont(std::get<1>(m_Style[1]));
      painter.drawText(
          std::get<2>(m_Style[1]).first, std::get<2>(m_Style[1]).second,
          QString::fromUtf8(reinterpret_cast<const char*>(
                                m_NetSpeedHelper->m_SysInfo[1].data()),
                            m_NetSpeedHelper->m_SysInfo[1].size()));
      painter.setPen(std::get<0>(m_Style[2]));
      painter.setFont(std::get<1>(m_Style[2]));
      painter.drawText(
          std::get<2>(m_Style[2]).first, std::get<2>(m_Style[2]).second,
          QString::fromUtf8(reinterpret_cast<const char*>(
                                m_NetSpeedHelper->m_SysInfo[2].data()),
                            m_NetSpeedHelper->m_SysInfo[2].size()));
      break;
  }
  painter.end();
}

void SpeedBox::showEvent(QShowEvent* event) {
  m_NetSpeedHelper->ClearData();
  m_Timer->start();
  event->accept();
}

void SpeedBox::hideEvent(QHideEvent* event) {
  m_Timer->stop();
  event->accept();
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
void SpeedBox::enterEvent(QEvent* event)
#else
void SpeedBox::enterEvent(QEnterEvent* event)
#endif
{
  const QRect& rect = geometry();
  m_Animation->setStartValue(geometry());
  if (m_Width != width()) {
    m_Animation->setDuration(200);
    if (rect.left() <= 0) {
      m_Animation->setEndValue(QRect(0, rect.top(), m_Width, m_Height));
    } else if (rect.right() + 1 >= m_ScreenWidth) {
      m_Animation->setEndValue(
          QRect(m_ScreenWidth - m_Width, rect.top(), m_Width, m_Height));
    } else {
      goto label;
    }
  } else if (m_Height != height()) {
    m_Animation->setDuration(100);
    if (rect.top() <= 0) {
      m_Animation->setEndValue(QRect(rect.left(), 0, m_Width, m_Height));
    } else if (rect.bottom() + 1 >= m_ScreenHeight) {
      m_Animation->setEndValue(
          QRect(rect.left(), m_ScreenHeight - m_Height, m_Width, m_Height));
    } else {
      goto label;
    }
  } else {
    goto label;
  }
  m_Animation->start();
label:
  if (event) event->accept();
}

void SpeedBox::leaveEvent(QEvent* event) {
  AutoHide();
  if (event) event->accept();
}

bool SpeedBox::eventFilter(QObject* object, QEvent* event) {
  if (object == this && event->type() == QEvent::WindowDeactivate) {
    m_ChangeMode = 0;
    return true;
  }
  return false;
}

void SpeedBox::AutoHide() {
  constexpr int liuchu = 2;
  const QRect& rect = geometry();
  m_Animation->setStartValue(geometry());
  if (rect.left() == 0) {
    m_Animation->setDuration(200);
    m_Animation->setEndValue(QRect(0, rect.top(), liuchu, m_Height));
    m_Side = SideType::Left;
  } else if (rect.right() + 1 >= m_ScreenWidth) {
    m_Animation->setDuration(200);
    m_Animation->setEndValue(
        QRect(m_ScreenWidth - liuchu, rect.top(), liuchu, m_Height));
    m_Side = SideType::Right;
  } else if (rect.top() == 0) {
    m_Animation->setDuration(100);
    m_Animation->setEndValue(QRect(rect.left(), 0, m_Width, liuchu));
    m_Side = SideType::Top;
  } else if (rect.bottom() + 1 >= m_ScreenHeight) {
    m_Animation->setDuration(100);
    m_Animation->setEndValue(
        QRect(rect.left(), m_ScreenHeight - liuchu, m_Width, liuchu));
    m_Side = SideType::Bottom;
  } else {
    m_Side = SideType::Right;
    return;
  }
  m_Animation->start();
}

void SpeedBox::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasFormat("text/uri-list"))
    event->acceptProposedAction();
}

void SpeedBox::dropEvent(QDropEvent* event) {
  QByteArray data = event->mimeData()->urls().first().toString().toUtf8();
  std::u8string temp(data.begin(), data.end());
  temp.erase(0, 7);
  m_VarBox->m_Wallpaper->SetDropFile(temp);
}

SpeedBox::SpeedBox(QWidget* parent)
    : QWidget(parent),
      m_Side(SideType::Right),
      m_ChangeMode(0),
      m_Width(100),
      m_Height(42),
      m_ScreenWidth(0),
      m_ScreenHeight(0),
      m_Animation(new QPropertyAnimation(this)),
      m_NetSpeedHelper(new NetSpeedHelper),
      m_Timer(new QTimer) {
  QRect geo = QGuiApplication::primaryScreen()->geometry();
  *const_cast<int*>(&m_ScreenWidth) = geo.width();
  *const_cast<int*>(&m_ScreenHeight) = geo.height();
  m_Animation->setTargetObject(this);
  m_Animation->setPropertyName("geometry");
  m_Timer->setInterval(1000);
  connect(m_Timer, &QTimer::timeout, this, &SpeedBox::OnTimer);
  m_Timer->start();
  QWidget::installEventFilter(this);
  //     connect(m_Animation, &QPropertyAnimation::finished,
  //     this, &SpeedBox::WritePosition);
  //     QGraphicsBlurEffect *blureffect = new QGraphicsBlurEffect(this);
  //     blureffect->setBlurRadius(5);	   //数值越大，越模糊
  //     setGraphicsEffect(blureffect);      //设置模糊特效
}

SpeedBox::~SpeedBox() {
  m_Timer->stop();
  delete m_Timer;
  delete m_NetSpeedHelper;
}

void SpeedBox::SetupUi() {
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
  setAttribute(Qt::WA_TranslucentBackground);
  setMaximumSize(m_Width, m_Height);
  setToolTip("行動是治癒恐懼的良藥，而猶豫、拖延將不斷滋養恐懼");
  setCursor(Qt::CursorShape::PointingHandCursor);
  setAcceptDrops(true);
  ReadPosition();
  GetBackGroundColor();
  connect(m_VarBox->m_Menu, &SpeedMenu::ChangeBoxColor, this,
          &SpeedBox::SetBackGroundColor);
  connect(m_VarBox->m_Menu, &SpeedMenu::ChangeBoxAlpha, this,
          &SpeedBox::SetBackGroundAlpha);
  GetStyle();
}

void SpeedBox::GetStyle() {
  const char8_t* li[] = {u8"MemUseage", u8"NetUpSpeed", u8"NetDownSpeed"};
  auto& ui = m_GlobalSetting->find(u8"FormUi")->second;
  for (size_t i = 0; i < 3; ++i) {
    auto& ptr = ui[li[i]].second;
    std::u8string_view str = ptr[u8"color"].second.getValueString();
    std::get<0>(m_Style[i])
        .setNamedColor(QString::fromUtf8(
            reinterpret_cast<const char*>(str.data()), str.size()));
    str = ptr[u8"font-family"].second.getValueString();
    std::get<1>(m_Style[i])
        .setFamily(QString::fromUtf8(reinterpret_cast<const char*>(str.data()),
                                     str.size()));
    std::get<1>(m_Style[i])
        .setPointSize(ptr[u8"font-size"].second.getValueInt());
    std::get<1>(m_Style[i]).setItalic(ptr[u8"italic"].second.isTrue());
    std::get<1>(m_Style[i]).setBold(ptr[u8"bold"].second.isTrue());
    std::get<2>(m_Style[i]).first = ptr[u8"pos"].second[0].getValueInt();
    std::get<2>(m_Style[i]).second = ptr[u8"pos"].second[1].getValueInt();
  }
}

void SpeedBox::SaveStyle() {
  const char8_t* li[] = {u8"MemUseage", u8"NetUpSpeed", u8"NetDownSpeed"};
  if (m_ChangeMode) m_ChangeMode = 0;
  auto& ui = m_GlobalSetting->find(u8"FormUi")->second;
  for (size_t i = 0; i < 3; ++i) {
    auto& ptr = ui[li[i]].second;
    QByteArray array = std::get<0>(m_Style[i]).name().toUtf8();
    ptr[u8"color"].second.getValueString().assign(array.begin(), array.end());
    array = std::get<1>(m_Style[i]).family().toUtf8();
    ptr[u8"font-family"].second.getValueString().assign(array.begin(),
                                                        array.end());
    ptr[u8"font-size"].second.setValue(std::get<1>(m_Style[i]).pointSize());
    ptr[u8"italic"].second.setValue(std::get<1>(m_Style[i]).italic());
    ptr[u8"bold"].second.setValue(std::get<1>(m_Style[i]).bold());
    ptr[u8"pos"].second.getArray().assign(
        {std::get<2>(m_Style[i]).first, std::get<2>(m_Style[i]).second});
  }
  m_GlobalSetting->toFile(m_szClobalSettingFile);
}

void SpeedBox::ReadPosition() {
  int pt[2]{0, 0};
  FILE* fp = fopen("boxpos.bin", "rb");
  if (!fp) {
    setGeometry(30, 30, m_Width, m_Height);
    return;
  }
  fread(pt, sizeof(int), 2, fp);
  fclose(fp);
  setGeometry(pt[0], pt[1], m_Width, m_Height);
  AutoHide();
}

void SpeedBox::WritePosition() {
  int pt[2]{x(), y()};
  FILE* fp = fopen("boxpos.bin", "wb");
  if (!fp) return;
  fwrite(pt, sizeof(int), 2, fp);
  fclose(fp);
}

void SpeedBox::GetBackGroundColor() {
  auto ptr = m_GlobalSetting->find(u8"FormUi")
                 ->second.find(u8"BkColor")
                 ->second.beginA();
  m_BackCol.setRed(ptr->getValueInt());
  m_BackCol.setGreen((++ptr)->getValueInt());
  m_BackCol.setBlue((++ptr)->getValueInt());
  m_BackCol.setAlpha((++ptr)->getValueInt());
}

void SpeedBox::OnTimer() {
  m_NetSpeedHelper->GetSysInfo();
  update();
}

void SpeedBox::SetBackGroundColor(QColor col) {
  auto& array = m_GlobalSetting->find(u8"FormUi")
                    ->second.find(u8"BkColor")
                    ->second.getArray();
  col.setAlpha(array.back().getValueInt());
  m_BackCol = std::move(col);
  array.assign({m_BackCol.red(), m_BackCol.green(), m_BackCol.blue(),
                std::move(array.back())});
  m_GlobalSetting->toFile(m_szClobalSettingFile);
}

void SpeedBox::SetBackGroundAlpha(int alpha) {
  m_BackCol.setAlpha(alpha);
  m_GlobalSetting->find(u8"FormUi")
      ->second.find(u8"BkColor")
      ->second.backA()
      .setValue(alpha);
  m_GlobalSetting->toFile(m_szClobalSettingFile);
}
