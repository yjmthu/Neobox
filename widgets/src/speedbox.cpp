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
#include <shortcut.h>
#include <speedbox.h>
#include <translatedlg.h>
#include <varbox.h>
#include <wallpaper.h>
#include <yjson.h>
#include <appcode.hpp>

#include <array>
#include <filesystem>
#include <ranges>

#ifdef _WIN32
#include <Windows.h>
#endif

SpeedBox::SpeedBox(QWidget* parent)
    : QWidget(parent,
              Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool),
      m_NetSpeedHelper(new NetSpeedHelper),
      m_Timer(new QTimer(this)),
      m_MainMenu(nullptr),
      m_Animation(new QPropertyAnimation(this, "geometry"))
{
  SetWindowMode();
  SetStyleSheet();
  SetHideFullScreen();
  SetBaseLayout();
  m_NetSpeedHelper->InitStrings();
  UpdateTextContent();
  connect(m_Timer, &QTimer::timeout, this, [this]() {
    m_NetSpeedHelper->GetSysInfo();
    UpdateTextContent();
  });
  m_Timer->start(1000);
}

SpeedBox::~SpeedBox() {
  m_Timer->stop();
  delete m_NetSpeedHelper;
}

void SpeedBox::Show() {
  show();

  /*
   * Notice
   * m_MainMenu should be construct after SpeedBox isShown.
   */

  if (m_MainMenu)
    return;
  m_MainMenu = new NeoMenu(this);  // must be this, not m_CentralWidget.
  m_MainMenu->SetFormColorEffect();

  m_Animation->setDuration(100);
  m_Animation->setTargetObject(this);
  connect(m_Animation, &QPropertyAnimation::finished, this, [this]() {
    VarBox::GetSettings(u8"FormGlobal")[u8"Position"].getArray() =
        YJson::A{x(), y()};
    VarBox::WriteSettings();
  });
}

void SpeedBox::SetWindowMode() {
  setWindowTitle("Neobox");
  setAttribute(Qt::WA_TranslucentBackground, true);
  // setAttribute(Qt::WA_DeleteOnClose, true);
  setAcceptDrops(true);

  const auto& position =
      VarBox::GetSettings(u8"FormGlobal")[u8"Position"].getArray();
  move(position.front().getValueInt(), position.back().getValueInt());
}

void SpeedBox::SetStyleSheet() {
  QFile fStyle(QStringLiteral("styles/AppStyle.css"));
  if (fStyle.open(QIODevice::ReadOnly)) {
    setStyleSheet(fStyle.readAll());
    fStyle.close();
  }
  setWindowIcon(QIcon(":/icons/neobox.ico"));
}

void SpeedBox::UpdateSkin()
{
  m_CentralWidget->deleteLater();
  SetBaseLayout();
  UpdateTextContent();
  m_CentralWidget->show();
}

void SpeedBox::SetBaseLayout() {

  QUiLoader loader;
  const auto& skins = VarBox::GetInstance()->m_Skins;
  const auto& skinName = VarBox::GetSettings(u8"FormGlobal")[u8"CurSkin"].getValueString();
  const auto& u8StringPath = std::find_if(skins.cbegin(), skins.cend(), [&skinName](const VarBox::Skin& item){
    return item.name == skinName;
  })->path;
  QFile file(QString::fromUtf8(u8StringPath.data(), u8StringPath.size()));
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
    QByteArray buffer = m_MemColorFrame->styleSheet().toUtf8();
    m_MemFrameStyle.assign(buffer.begin(), buffer.end());
    const std::string style = std::vformat(m_MemFrameStyle, std::make_format_args(
        0.1, 0.2, 0.8, 0.9
    ));
    m_MemColorFrame->setStyleSheet(QString::fromUtf8(style.data(), style.size()));
  }
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
    if (m_MainMenu->m_TranslateDlg->isVisible()) {
      m_MainMenu->m_TranslateDlg->hide();
    }
  }
}

void SpeedBox::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    m_ConstPos = event->pos();
    setMouseTracking(true);
  } else if (event->button() == Qt::RightButton) {
    m_MainMenu->popup(pos() + event->pos());
  } else if (event->button() == Qt::MiddleButton) {
    VarBox::GetInstance()->m_SharedMemory->detach();
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList {});
    qApp->quit();
  }
}

void SpeedBox::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    setMouseTracking(false);
    VarBox::GetSettings(u8"FormGlobal")[u8"Position"].getArray() =
        YJson::A{x(), y()};
    VarBox::WriteSettings();
  }
}

void SpeedBox::mouseDoubleClickEvent(QMouseEvent* event) {
  if (m_MainMenu->m_TranslateDlg->isVisible()) {
    m_MainMenu->m_TranslateDlg->hide();
  } else {
    m_MainMenu->m_TranslateDlg->Show(frameGeometry());
  }
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
  namespace fs = std::filesystem;
  auto mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    const auto&& urls =
        mimeData->urls() | std::views::transform([](const QUrl& i) {
          return fs::path(i.toLocalFile().toStdWString());
        });
    m_MainMenu->m_Wallpaper->SetDropFile(
        std::deque<fs::path>(urls.begin(), urls.end()));
    event->accept();
  } else if (mimeData->hasText()) {
    const QString text = mimeData->text();
    if (!text.isEmpty()) {
      m_MainMenu->m_TranslateDlg->Show(frameGeometry(), mimeData->text());
    } else {
      m_MainMenu->m_TranslateDlg->Show(frameGeometry());
    }
  } else {
    event->ignore();
  }
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
    m_MainMenu->m_Shortcut->CallFunction(msg->wParam);
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
  if (rtForm.right() + delta >= rtScreen.right()) {
    m_HideSide = HideSide::Right;
    rtForm.moveTo(rtScreen.right() - delta, rtForm.y());
  } else if (rtForm.left() - delta <= rtScreen.left()) {
    m_HideSide = HideSide::Left;
    rtForm.moveTo(delta - rtForm.width(), rtForm.y());
  } else if (rtForm.top() - delta <= rtScreen.top()) {
    m_HideSide = HideSide::Top;
    rtForm.moveTo(rtForm.x(), delta - rtForm.height());
  } else if (rtForm.bottom() + delta >= rtScreen.bottom()) {
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

void SpeedBox::Move()
{
  move(100, 100);
  m_HideSide = HideSide::None;
  VarBox::GetSettings(u8"FormGlobal")[u8"Position"].getArray() = YJson::A{100, 100};
  VarBox::WriteSettings();
  if (!isVisible())
    show();
  VarBox::ShowMsg("移动成功！");
}
