#include <QApplication>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QTimer>
#include <QVBoxLayout>

#include <neomenu.h>
#include <speedbox.h>
#include <varbox.h>
#include <wallpaper.h>
#include <yjson.h>
#include <appcode.hpp>
#include <netspeedhelper.h>
#include <translatedlg.h>
#include <shortcut.h>

#include <filesystem>
#include <ranges>
#include <array>

#ifdef _WIN32
#include <Windows.h>
#endif

SpeedBox::SpeedBox(QWidget* parent)
    : QWidget(parent,
              Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool),
      m_CentralWidget(new QWidget(this)),
      m_NetSpeedHelper(new NetSpeedHelper),
      m_TextMemUseage(new QLabel(m_CentralWidget)),
      m_TextUploadSpeed(new QLabel(m_CentralWidget)),
      m_TextDownLoadSpeed(new QLabel(m_CentralWidget)),
      m_Timer(new QTimer(this)),
      m_MainMenu(nullptr)
{
  SetWindowMode();
  SetBaseLayout();
  SetStyleSheet();
  SetHideFullScreen();
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
  if (!m_MainMenu) {
    m_MainMenu = new NeoMenu(this);  // must be this, not m_CentralWidget.
    m_MainMenu->SetFormColorEffect();
  }
}

void SpeedBox::SetWindowMode() {
  setAttribute(Qt::WA_TranslucentBackground, true);
  setAttribute(Qt::WA_DeleteOnClose, true);
  setAcceptDrops(true);
  setMinimumSize(100, 40);
  setMaximumSize(100, 40);
  m_CentralWidget->setMinimumSize(100, 40);
  m_CentralWidget->setMaximumSize(100, 40);
  m_CentralWidget->setObjectName("center");
  m_TextMemUseage->setObjectName("memUse");
  m_TextDownLoadSpeed->setObjectName("netDown");
  m_TextUploadSpeed->setObjectName("netUp");
  m_TextMemUseage->setMinimumWidth(30);
  m_TextUploadSpeed->setMinimumWidth(60);
  m_TextDownLoadSpeed->setMinimumWidth(60);
  const auto& array =
      VarBox::GetSettings(u8"FormGlobal")[u8"Position"].second.getArray();
  move(array.front().getValueInt(), array.back().getValueInt());
}

void SpeedBox::SetStyleSheet() {
  QFile fStyle(QStringLiteral("styles/MenuStyle.css"));
  if (fStyle.open(QIODevice::ReadOnly)) {
    setStyleSheet(fStyle.readAll());
    fStyle.close();
  }
  setWindowIcon(QIcon(":/icons/speedbox.ico"));
  setCursor(Qt::PointingHandCursor);
  std::u8string& toolTip =
      VarBox::GetSettings(u8"FormGlobal")[u8"ToolTip"].second.getValueString();
  setToolTip(QString::fromUtf8(toolTip.data(), toolTip.size()));
}

void SpeedBox::SetBaseLayout() {
  YJson& jsFormUi = VarBox::GetSettings(u8"FormUi");
  QByteArray qByteName;
  std::array<QLabel*, 3> labels = {
    m_TextMemUseage, m_TextUploadSpeed, m_TextDownLoadSpeed
  };
  for (auto label: labels) {
    qByteName = label->objectName().toUtf8();
    YJson& temp = jsFormUi[std::u8string(qByteName.begin(), qByteName.end())].second;
    const auto& jsPos = temp[u8"Pos"].second.getArray();
    label->move(jsPos.front().getValueInt(), jsPos.back().getValueInt());
    // 
  }
}

void SpeedBox::UpdateTextContent() {
  m_TextMemUseage->setText(
      QString::number(std::get<0>(m_NetSpeedHelper->m_SysInfo)));
  m_TextUploadSpeed->setText(
      QString::fromStdWString(m_NetSpeedHelper->FormatSpped(
          std::get<1>(m_NetSpeedHelper->m_SysInfo), true)));
  m_TextDownLoadSpeed->setText(
      QString::fromStdWString(m_NetSpeedHelper->FormatSpped(
          std::get<2>(m_NetSpeedHelper->m_SysInfo), false)));
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
    qApp->exit((int)ExitCode::RETCODE_RESTART);
  }
}

void SpeedBox::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    setMouseTracking(false);
    VarBox::GetSettings(u8"FormGlobal")[u8"Position"].second.getArray() =
        YJson::A{x(), y()};
    VarBox::WriteSettings();
  }
}

void SpeedBox::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (m_MainMenu->m_TranslateDlg->isVisible()) {
    m_MainMenu->m_TranslateDlg->hide();
  } else {
    m_MainMenu->m_TranslateDlg->Show(frameGeometry());
  }
  event->accept();
}

void SpeedBox::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls())
    event->acceptProposedAction();
  else
    event->ignore();
}

void SpeedBox::dropEvent(QDropEvent* event) {
  namespace fs = std::filesystem;
  if (event->mimeData()->hasUrls()) {
    auto urls =
        event->mimeData()->urls() | std::views::transform([](const QUrl& i) {
          return fs::path(i.toLocalFile().toStdWString());
        });
    m_MainMenu->m_Wallpaper->SetDropFile(
        std::deque<fs::path>(urls.begin(), urls.end()));
    event->accept();
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
