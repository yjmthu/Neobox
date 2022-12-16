#include <QApplication>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>
#include <QUiLoader>
#include <QSharedMemory>
#include <QProcess>

#include <neomenu.h>
#include <netspeedhelper.h>
#include <speedbox.h>
#include <yjson.h>
#include <pluginmgr.h>
#include <appcode.hpp>
#include <pluginobject.h>
#include <neoapp.h>

#include <array>
#include <filesystem>
#include <ranges>

#ifdef _WIN32
#include <Windows.h>
#endif

extern PluginMgr* mgr;
extern GlbObject* glb;

SpeedBox::SpeedBox(PluginObject* plugin, YJson& settings, QMenu* netcardMenu)
    : QWidget(/*glb->glbGetMenu()*/ nullptr,
              Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool),
    m_PluginObject(plugin),
    m_Settings(settings),
    m_NetCardMenu(*netcardMenu),
    m_NetSpeedHelper(new NetSpeedHelper),
    m_Timer(new QTimer(this)),
    m_Animation(new QPropertyAnimation(this, "geometry"))
{
  SetWindowMode();
  SetHideFullScreen();
  SetBaseLayout();
  InitNetCard();
}

SpeedBox::~SpeedBox() {
  m_Timer->stop();
  delete m_NetSpeedHelper;
}

void SpeedBox::InitShow() {
  show();

  /*
   * Notice
   * m_MainMenu should be construct after SpeedBox isShown.
   */

  // if (m_MainMenu)
  //   return;
  // m_MainMenu = new NeoMenu(this);  // must be this, not m_CentralWidget.
  // m_MainMenu->SetFormColorEffect();

  m_Animation->setDuration(100);
  m_Animation->setTargetObject(this);
  connect(m_Animation, &QPropertyAnimation::finished, this, [this]() {
    m_Settings[u8"Position"].getArray() =
        YJson::A{x(), y()};
    mgr->SaveSettings();
  });

  UpdateNetCardMenu();
}

void SpeedBox::SetWindowMode() {
  setWindowTitle("Neobox");
  setWindowIcon(QIcon(":/icons/neobox.ico"));
  setAttribute(Qt::WA_TranslucentBackground, true);
  // setAttribute(Qt::WA_DeleteOnClose, true);
  setAcceptDrops(true);

  const auto& position = m_Settings [u8"Position"].getArray();
  move(position.front().getValueInt(), position.back().getValueInt());
}

void SpeedBox::UpdateSkin()
{
  m_CentralWidget->hide();
  m_CentralWidget->deleteLater();
  SetBaseLayout();
  UpdateTextContent();
  m_CentralWidget->show();
}

void SpeedBox::SetBaseLayout() {

  QUiLoader loader(this);
  const auto& skinName = m_Settings[u8"CurSkin"].getValueString();
  QString qSkinPath = PluginObject::Utf82QString(m_Settings[u8"UserSkins"][skinName].getValueString());
  if (!QFile::exists(qSkinPath))
    throw nullptr;
  QFile file(qSkinPath);
  file.open(QFile::ReadOnly);
  m_CentralWidget = loader.load(&file, this);
  file.close();

  m_CentralWidget->move(0, 0);
  setMinimumSize(m_CentralWidget->size());
  setMaximumSize(m_CentralWidget->size());

  m_TextMemUseage = m_CentralWidget->findChild<QLabel*>("memUse");
  m_TextUploadSpeed = m_CentralWidget->findChild<QLabel*>("netUp");
  m_TextDownloadSpeed = m_CentralWidget->findChild<QLabel*>("netDown");
  m_TextCpuUseage = m_CentralWidget->findChild<QLabel*>("cpuUse");

  QByteArray buffer;
  if (m_TextUploadSpeed) {
    buffer = m_TextUploadSpeed->text().toUtf8();
    m_NetSpeedHelper->m_StrFmt[0].assign(buffer.begin(), buffer.end());
  }
  if (m_TextDownloadSpeed) {
    buffer = m_TextDownloadSpeed->text().toUtf8();
    m_NetSpeedHelper->m_StrFmt[1].assign(buffer.begin(), buffer.end());
  }
  if (m_TextMemUseage) {
    buffer = m_TextMemUseage->text().toUtf8();
    m_NetSpeedHelper->m_StrFmt[2].assign(buffer.begin(), buffer.end());
  }
  if (m_TextCpuUseage) {
    buffer = m_TextCpuUseage->text().toUtf8();
    m_NetSpeedHelper->m_StrFmt[3].assign(buffer.begin(), buffer.end());
  }

  m_MemColorFrame = m_CentralWidget->findChild<QFrame*>("memColorFrame");
  if (m_MemColorFrame != nullptr) {
    buffer = m_MemColorFrame->styleSheet().toUtf8();
    m_MemFrameStyle.assign(buffer.begin(), buffer.end());
    const std::string style = std::vformat(m_MemFrameStyle, std::make_format_args(
        0.1, 0.2, 0.8, 0.9
    ));
    m_MemColorFrame->setStyleSheet(QString::fromUtf8(style.data(), style.size()));
  }

  setStyleSheet(glb->glbGetMenu()->styleSheet());
}

void SpeedBox::UpdateTextContent() {
  static const auto qstr = [](const std::string& str) {
    return QString::fromUtf8(str.data(), str.size());
  };

  auto iter = m_NetSpeedHelper->m_SysInfo.cbegin();
  if (m_TextUploadSpeed)
    m_TextUploadSpeed->setText(qstr(iter[0]));
  if (m_TextDownloadSpeed)
    m_TextDownloadSpeed->setText(qstr(iter[1]));
  if (m_TextMemUseage)
    m_TextMemUseage->setText(qstr(iter[2]));
  if (m_TextCpuUseage)
    m_TextCpuUseage->setText(qstr(iter[3]));

  if (m_MemColorFrame != nullptr) {
    const float x1 = m_NetSpeedHelper->m_MemUse > 0.02 ? m_NetSpeedHelper->m_MemUse - 0.02 : m_NetSpeedHelper->m_MemUse;
    const float x2 = x1 + 0.02, y1 = 1 - x2, y2 = 1 - x1;
    const std::string style = std::vformat(m_MemFrameStyle, std::make_format_args(
        x1, x2, y1, y2
    ));
    m_MemColorFrame->setStyleSheet(QString::fromUtf8(style.data(), style.size()));
  }
}

void SpeedBox::SetHideFullScreen() {
  static APPBARDATA abd{0};
  abd.cbSize = sizeof(APPBARDATA);
  abd.hWnd = reinterpret_cast<HWND>(winId());
  abd.uCallbackMessage = static_cast<UINT>(MsgCode::MSG_APPBAR_MSGID);
  SHAppBarMessage(ABM_NEW, &abd);
}

void SpeedBox::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::LeftButton) {
    move(pos() + event->pos() - m_ConstPos);
    m_PluginObject->SendBroadcast(PluginEvent::MouseMove, event);
  }
}

void SpeedBox::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    m_ConstPos = event->pos();
    setMouseTracking(true);
  } else if (event->button() == Qt::RightButton) {
    glb->glbGetMenu()->popup(pos() + event->pos());
  } else if (event->button() == Qt::MiddleButton) {
    // glb->glbWriteSharedFlag(1);
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList {});
    qApp->quit();
  }
}

void SpeedBox::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    setMouseTracking(false);
    m_Settings[u8"Position"].getArray() =
        YJson::A{x(), y()};
    mgr->SaveSettings();
  }
}

void SpeedBox::mouseDoubleClickEvent(QMouseEvent* event) {
  m_PluginObject->SendBroadcast(PluginEvent::MouseDoubleClick, event);
  event->accept();
}

void SpeedBox::dragEnterEvent(QDragEnterEvent* event) {
  auto mimeData = event->mimeData();
  if (mimeData->hasUrls() || mimeData->hasText())
    event->acceptProposedAction();
  else
    event->ignore();
}

void SpeedBox::dropEvent(QDropEvent* event) {
  auto mimeData = event->mimeData();
  // if (mimeData->hasUrls()) {}

  /*
  auto ocrUrlsView = urls | std::views::filter([](const QUrl& i) {
    return i.isValid() && i.fileName().endsWith(".ui") || i.fileName().endsWith(".traineddata"); });
    */

  m_PluginObject->SendBroadcast(PluginEvent::Drop, event);
  event->accept();
}

bool SpeedBox::nativeEvent(const QByteArray& eventType,
                           void* message,
                           qintptr* result) {
  MSG* msg = static_cast<MSG*>(message);

  if (MsgCode::MSG_APPBAR_MSGID == static_cast<MsgCode>(msg->message)) {
    switch ((UINT)msg->wParam) {
      case ABN_FULLSCREENAPP:
        if (msg->lParam)
          hide();
        else
          show();
        return true;
      default:
        break;
    }
  } else if (WM_HOTKEY == msg->message) {
    /*
     * idHotKey = wParam;
     * Modifiers = (UINT) LOWORD(lParam);
     * uVirtKey = (UINT) HIWORD(lParam);
     */
    // m_MainMenu->m_Shortcut->CallFunction(msg->wParam);
  }
  return false;
}

constexpr int delta = 1;

void SpeedBox::enterEvent(QEnterEvent* event) {
  auto rtScreen = QGuiApplication::primaryScreen()->geometry();
  auto rtForm = this->frameGeometry();
  m_Animation->setStartValue(rtForm);
  switch (m_HideSide) {
    case HideSide::Left:
      rtForm.moveTo(-delta, rtForm.y());
      break;
    case HideSide::Right:
      rtForm.moveTo(rtScreen.right() - rtForm.width() + delta, rtForm.y());
      break;
    case HideSide::Top:
      rtForm.moveTo(rtForm.x(), -delta);
      break;
    case HideSide::Bottom:
      rtForm.moveTo(rtForm.x(), rtScreen.bottom() - rtForm.height() + delta);
      break;
    default:
      event->ignore();
      return;
  }
  event->accept();
  m_Animation->setEndValue(rtForm);
  m_Animation->start();
}

void SpeedBox::leaveEvent(QEvent* event) {
  auto rtScreen = QGuiApplication::primaryScreen()->geometry();
  auto rtForm = this->frameGeometry();
  m_Animation->setStartValue(rtForm);
  auto const hideBits = m_Settings[u8"HideAside"].getValueInt();
  if ((HideSide::Right & hideBits) && rtForm.right() + delta >= rtScreen.right()) {
    m_HideSide = HideSide::Right;
    rtForm.moveTo(rtScreen.right() - delta, rtForm.y());
  } else if ((HideSide::Left & hideBits)  && rtForm.left() - delta <= rtScreen.left()) {
    m_HideSide = HideSide::Left;
    rtForm.moveTo(delta - rtForm.width(), rtForm.y());
  } else if ((HideSide::Top & hideBits) && rtForm.top() - delta <= rtScreen.top()) {
    m_HideSide = HideSide::Top;
    rtForm.moveTo(rtForm.x(), delta - rtForm.height());
  } else if ((HideSide::Bottom & hideBits) && rtForm.bottom() + delta >= rtScreen.bottom()) {
    m_HideSide = HideSide::Bottom;
    rtForm.moveTo(rtForm.x(), rtScreen.bottom() - delta);
  } else {
    m_HideSide = HideSide::None;
    event->ignore();
    return;
  }
  m_Animation->setEndValue(rtForm);
  m_Animation->start();
  event->accept();
}

void SpeedBox::InitMove()
{
  move(100, 100);
  m_HideSide = HideSide::None;
  m_Settings[u8"Position"].getArray() = YJson::A{100, 100};
  mgr->SaveSettings();
  if (!isVisible())
    show();
  glb->glbShowMsg("移动成功！");
}


void SpeedBox::InitNetCard()
{
  const auto& blacklistView = 
    m_Settings[u8"NetCardDisabled"].getArray() |
    std::views::transform([](const YJson& item)->const std::u8string&
      { return item.getValueString(); });
  for (auto i: blacklistView) {
    const QByteArray array = QString::fromUtf8(i.data(), i.size()).toLocal8Bit();
    m_NetSpeedHelper->m_AdapterBalckList.emplace(array.begin(), array.end());
  }

  m_NetSpeedHelper->InitStrings();
  UpdateTextContent();
  connect(m_Timer, &QTimer::timeout, this, [this]() {
    static int count = 10;
    if (--count == 0) {
      m_NetSpeedHelper->UpdateAdaptersAddresses();
      UpdateNetCardMenu();
      count = 10;
    }
    m_NetSpeedHelper->GetSysInfo();
    UpdateTextContent();
#if 0
    const auto flag = glb->glbReadSharedFlag();
    if (flag == 2) {
      glb->glbWriteSharedFlag(0);
      InitMove();
    } else if (flag == 4) {
      QApplication::quit();
    }
#endif
  });
  m_Timer->start(1000);
}

void SpeedBox::UpdateNetCardMenu()
{
  // auto menu = m_MainMenu->m_ExMenus[u8"NetCardSelect"];
  m_NetCardMenu.clear();
  for (const auto& i: m_NetSpeedHelper->m_Adapters) {
    const QString fName = QString::fromWCharArray(i.friendlyName.data(), i.friendlyName.size());
    const QString aName = QString::fromLocal8Bit(i.adapterName.data(), i.adapterName.size());
    auto action = m_NetCardMenu.addAction(fName);
    action->setCheckable(true);
    action->setChecked(i.enabled);
    action->setToolTip(aName);
    action->setProperty("guid", aName);
    connect(action, &QAction::triggered, this, std::bind(
      &SpeedBox::UpdateNetCard, this, action, std::placeholders::_1
    ));
  }
}

void SpeedBox::UpdateNetCard(QAction* action, bool checked)
{
  const QString data = action->property("guid").toString();
  QByteArray buffer = data.toLocal8Bit();
  std::string guid(buffer.cbegin(), buffer.cend());
  buffer = data.toUtf8();
  std::u8string name(buffer.cbegin(), buffer.cend());

  if (checked) {
    m_Settings[u8"NetCardDisabled"].removeByValA(name);
    m_NetSpeedHelper->m_AdapterBalckList.erase(guid);
    glb->glbShowMsg("添加网卡成功！"); // removed from blacklist
  } else {
    m_Settings[u8"NetCardDisabled"].append(std::move(name));
    m_NetSpeedHelper->m_AdapterBalckList.emplace(std::move(guid));
    glb->glbShowMsg("删除网卡成功！");
  }
  m_NetSpeedHelper->UpdateAdaptersAddresses();
  mgr->SaveSettings();
}
