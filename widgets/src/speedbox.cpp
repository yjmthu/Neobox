#include <QApplication>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QMimeData>
#include <QMessageBox>

#include <neomenu.h>
#include <speedbox.h>
#include <varbox.h>
#include <yjson.h>
#include <wallpaper.h>
#include <appcode.hpp>

#include <filesystem>
#include <ranges>

SpeedBox::SpeedBox(QWidget* parent)
    : QWidget(parent,
              Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool),
      m_CentralWidget(new QWidget(this)),
      m_TextMemUseage(new QLabel),
      m_TextUploadSpeed(new QLabel),
      m_TextDownLoadSpeed(new QLabel),
      m_Timer(new QTimer(this)),
      m_MainMenu(new NeoMenu(this)) {
  SetWindowMode();
  SetBaseLayout();
  SetStyleSheet();
  UpdateTextContent();
  connect(m_Timer, &QTimer::timeout, this, [this]() {
    m_NetSpeedHelper.GetSysInfo();
    UpdateTextContent();
  });
  m_Timer->start(1000);
}

SpeedBox::~SpeedBox() {
  m_Timer->stop();
  // delete m_MainMenu;
  delete m_TextDownLoadSpeed;
  delete m_TextUploadSpeed;
  delete m_TextMemUseage;
  delete m_CentralWidget;
}

void SpeedBox::SetWindowMode() {
  setAttribute(Qt::WA_TranslucentBackground);
  setAcceptDrops(true);
  setMinimumSize(100, 40);
  setMaximumSize(100, 40);
  m_CentralWidget->setMinimumSize(100, 40);
  m_CentralWidget->setMaximumSize(100, 40);
  m_CentralWidget->setObjectName("center");
  m_TextMemUseage->setObjectName("memUse");
  m_TextDownLoadSpeed->setObjectName("netDown");
  m_TextUploadSpeed->setObjectName("netUp");
  m_TextMemUseage->setMinimumWidth(26);
  m_TextMemUseage->setMaximumWidth(26);
  const auto& array =
      VarBox::GetSettings(u8"FormGlobal")[u8"Position"].second.getArray();
  move(array.front().getValueInt(), array.back().getValueInt());
}

void SpeedBox::SetStyleSheet() {
  QFile fStyle(QStringLiteral("styles/MenuStyle.css"));
  if (fStyle.open(QIODevice::ReadOnly)) {
    QByteArray&& array = fStyle.readAll();
    setStyleSheet(array);
    fStyle.close();
  }
  setCursor(Qt::PointingHandCursor);
  std::u8string& toolTip =
      VarBox::GetSettings(u8"FormGlobal")[u8"ToolTip"].second.getValueString();
  setToolTip(QString::fromUtf8(toolTip.data(), toolTip.size()));
}

void SpeedBox::SetBaseLayout() {
  QHBoxLayout* hout = new QHBoxLayout(m_CentralWidget);
  QVBoxLayout* vout = new QVBoxLayout;
  hout->setContentsMargins(6, 4, 4, 0);
  hout->setSpacing(0);
  vout->setSpacing(0);
  hout->addWidget(m_TextMemUseage);
  vout->addWidget(m_TextUploadSpeed);
  vout->addWidget(m_TextDownLoadSpeed);
  hout->addLayout(vout);
  m_CentralWidget->setLayout(hout);
  m_CentralWidget->move(0, 0);
}

void SpeedBox::UpdateTextContent() {
  m_TextMemUseage->setText(
      QString::number(std::get<0>(m_NetSpeedHelper.m_SysInfo)));
  m_TextUploadSpeed->setText(
      QString::fromStdWString(m_NetSpeedHelper.FormatSpped(
          std::get<1>(m_NetSpeedHelper.m_SysInfo), true)));
  m_TextDownLoadSpeed->setText(
      QString::fromStdWString(m_NetSpeedHelper.FormatSpped(
          std::get<2>(m_NetSpeedHelper.m_SysInfo), false)));
}

void SpeedBox::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::LeftButton) {
    move(pos() + event->pos() - m_ConstPos);
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
    VarBox::GetSettings(u8"FormGlobal")[u8"Position"].second.getArray()
      = YJson::A{x(), y()};
    VarBox::WriteSettings();
  }
}

void SpeedBox::dragEnterEvent(QDragEnterEvent* event)
{
  if(event->mimeData()->hasUrls())
    event->acceptProposedAction();
  else
    event->ignore();
}

void SpeedBox::dropEvent(QDropEvent* event)
{
  namespace fs = std::filesystem;
  if (event->mimeData()->hasUrls())
  {
    auto urls = event->mimeData()->urls() | std::views::transform([](const QUrl& i){
        return fs::path(i.toLocalFile().toStdWString());});
    m_MainMenu->m_Wallpaper->SetDropFile(std::deque<fs::path>(urls.begin(), urls.end()));
  }
}
