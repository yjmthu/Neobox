#include <usbdlg.h>
#include <yjson.h>
#include <usbdlgitem.h>
#include <pluginmgr.h>
#include <appcode.hpp>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QPropertyAnimation>

#include <windows.h>
#include <dbt.h>

UsbDlg::ItemMap UsbDlg::m_Items;

UsbDlg::UsbDlg(YJson& settings)
  : QDialog(nullptr, Qt::FramelessWindowHint | Qt::Tool)
  , m_Settings(settings)
  , m_CenterWidget(new QWidget(this))
  , m_MainLayout(new QVBoxLayout(m_CenterWidget))
  , m_Animation(new QPropertyAnimation(this, "geometry"))
  , m_Position(settings[u8"Position"])
{
  setWindowFlag(Qt::WindowStaysOnTopHint, m_Settings[u8"StayOnTop"].isTrue());
  setWindowTitle("U盘助手");
  setAttribute(Qt::WA_TranslucentBackground, true);
  SetupUi();
  SetupAnimation();
  GetUsbInfo();
}

UsbDlg::~UsbDlg()
{
  delete m_CenterWidget;
  delete reinterpret_cast<APPBARDATA*>(m_AppBarData);
}

void UsbDlg::SetupUi()
{
  setMinimumWidth(300);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_CenterWidget);
  m_CenterWidget->setStyleSheet("QWidget { background-color: white; border-radius: 3px; }");

  auto const titleLayout = new QHBoxLayout;
  auto const label = new QLabel("U盘助手", this);
  auto const btnTop = new QPushButton(this);
  auto const btnClose = new QPushButton(this);

  layout->setSizeConstraint(QLayout::SetFixedSize);    // 自动调节大小
  m_MainLayout->addLayout(titleLayout);
  titleLayout->setSpacing(13);
  titleLayout->addWidget(label);
  titleLayout->addStretch();
  titleLayout->addWidget(btnTop);
  titleLayout->addWidget(btnClose);

  auto const sizeBtn = QSize(15, 15);
  btnTop->setCheckable(true);
  btnTop->setChecked(m_Settings[u8"StayOnTop"].isTrue());
  btnTop->setMinimumSize(sizeBtn);
  btnTop->setMaximumSize(sizeBtn);
  btnTop->setStyleSheet(
    "QPushButton {"
      "border-image: url(:/icons/usb-top-gray.png);"
    "}"
    "QPushButton:hover {"
      "border-image: url(:/icons/usb-top-red.png);"
    "}"
    "QPushButton:checked {"
      "border-image: url(:/icons/usb-top-orange.png);"
    "}"
  );

  btnClose->setMinimumSize(sizeBtn);
  btnClose->setMaximumSize(sizeBtn);
  btnClose->setStyleSheet(
    "border-image: url(:/icons/usb-hide.png);"
  );

  connect(btnClose, &QPushButton::clicked, this, &QDialog::hide);
  connect(btnTop, &QPushButton::clicked, this, [this](bool on) {
    m_Settings[u8"StayOnTop"] = on;
    setWindowFlag(Qt::WindowStaysOnTopHint, on);
    show();
    mgr->SaveSettings();
  });
}

void UsbDlg::SetupAnimation()
{
  m_Animation->setDuration(100);
  m_Animation->setTargetObject(this);
  connect(m_Animation, &QPropertyAnimation::finished, this, [this]() {
    m_Position = YJson::A{x(), y()};
    mgr->SaveSettings();
  });
  if (m_Position.isArray()) {
    move(m_Position[0].getValueInt(), m_Position[1].getValueInt());
  }
  SetHideFullScreen();
}

void UsbDlg::GetUsbInfo()
{
  wchar_t disk_path[] {
    L' ', L':', L'\0'
  }; 
  wchar_t device_path[] = {
    L'\\', L'\\', L'.', L'\\',
    L' ', L':', L'\0'
  };        
		
	wchar_t i = L'A';
	DWORD bytes_returned = 0;
	STORAGE_DEVICE_NUMBER device_num;

  // ++i 放在前面，防止 continue 跳过
	for (DWORD all_disk = GetLogicalDrives(); all_disk; (all_disk >>= 1), ++i)
  {
		if (!(all_disk & 1)) continue;

    disk_path[0] = i;
    device_path[4] = i;
    if (GetDriveTypeW(disk_path) != DRIVE_REMOVABLE) continue;

    // get this usb device id
    HANDLE hDevice = CreateFileW(device_path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) continue;

    if (DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
              &device_num, sizeof(device_num), 
              &bytes_returned, (LPOVERLAPPED) NULL))
    {
      // usb_list[usb_device_cnt].device_num = device_num.DeviceNumber;
      auto const item = new UsbDlgItem(this, i, m_Items);
      m_MainLayout->addWidget(item);
      item->show();
    }
    CloseHandle(hDevice);
  }

  if (!m_Items.empty()) {
    show();
  }
}

std::string UsbDlg::GetDrives(const void* lpdb)
{
  auto const lpdbv = reinterpret_cast<const DEV_BROADCAST_VOLUME*>(lpdb);

  // https://learn.microsoft.com/en-us/windows/win32/api/dbt/ns-dbt-dev_broadcast_volume

  std::string drives;
  auto mask = lpdbv->dbcv_unitmask & ((1ul << 26) - 1);          // 去除高位，保证盘符在 ['A'-'Z'] 之间
  for (char i = 'A'; mask; ++i) {
    if (mask & 1)
      drives.push_back(i);
    mask >>= 1;
  }

  return drives;
}

void UsbDlg::DoDeviceArrival(const void* lpdb)
{
  for (auto c: GetDrives(lpdb)) {
    if (auto iter = m_Items.find(c); iter != m_Items.end())
      continue;
    auto const item = new UsbDlgItem(this, c, m_Items);
    m_MainLayout->addWidget(item);
    item->show();
  }
  if (m_Items.size() == 1 && !isVisible()) {
    show();
  }
}

void UsbDlg::DoDeviceRemoveComplete(const void* lpdb)
{
  for (auto c: GetDrives(lpdb)) {
    if (auto iter = m_Items.find(c); iter != m_Items.end()) {
      delete iter->second;
      // if (isVisible()) adjustSize();
    }
  }
  if (m_Items.empty() && isVisible()) {
    hide();
  }
}

void UsbDlg::SetHideFullScreen() {
  m_AppBarData = new APPBARDATA {
    sizeof(APPBARDATA),
    reinterpret_cast<HWND>(winId()), 
    static_cast<UINT>(MsgCode::MSG_APPBAR_MSGID),
    0, 0, 0
  };
  SHAppBarMessage(ABM_NEW, reinterpret_cast<APPBARDATA*>(m_AppBarData));
}

bool UsbDlg::nativeEvent(const QByteArray& eventType,
                           void* message,
                           qintptr* result) {
  static YJson& bHide = m_Settings[u8"HideWhenFull"];
  MSG* msg = static_cast<MSG*>(message);
  if (MsgCode::MSG_APPBAR_MSGID != static_cast<MsgCode>(msg->message)) return false;

  switch ((UINT)msg->wParam) {
    case ABN_FULLSCREENAPP:
      if (msg->lParam) {
        if (bHide.isTrue())
          hide();
      } else {
        show();
      }
      return true;
    default:
      break;
  }
  return false;
}

void UsbDlg::showEvent(QShowEvent* event)
{
  if (!m_Position.isArray()) {
    const auto size = QGuiApplication::primaryScreen()->size();
    const auto mSize = frameSize();
    move(
      size.width() - 30 - mSize.width(),
      70
    );
  }
  // adjustSize();
  event->accept();
}

void UsbDlg::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    m_ConstPos = event->pos();
    setMouseTracking(true);
  }
}

void UsbDlg::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    setMouseTracking(false);
    m_Position = YJson::A{x(), y()};
    mgr->SaveSettings();
  }
}

void UsbDlg::mouseMoveEvent(QMouseEvent* event)
{
  if (event->buttons() == Qt::LeftButton) {
    move(pos() + event->pos() - m_ConstPos);
  }
}

static constexpr int delta = 1;

void UsbDlg::enterEvent(QEnterEvent* event)
{
  if (m_Settings[u8"HideAside"].isFalse()) {
    event->ignore();
    return;
  }

  auto rtScreen = QGuiApplication::primaryScreen()->geometry();
  auto rtForm = this->frameGeometry();

  if (rtForm.left() < rtScreen.right() - delta)
  {
    event->ignore();
    return;
  }

  m_Animation->setStartValue(rtForm);
  rtForm.moveTo(rtScreen.right() - rtForm.width() + delta, rtForm.y());
  m_Animation->setEndValue(rtForm);
  m_Animation->start();

  event->accept();
}

void UsbDlg::leaveEvent(QEvent* event)
{
  if (m_Settings[u8"HideAside"].isFalse()) {
    event->ignore();
    return;
  }

  auto rtScreen = QGuiApplication::primaryScreen()->geometry();
  auto rtForm = this->frameGeometry();

  if (rtForm.right() + delta < rtScreen.right()) {
    event->ignore();
    return;
  }

  m_Animation->setStartValue(rtForm);

  rtForm.moveTo(rtScreen.right() - delta, rtForm.y());

  m_Animation->setEndValue(rtForm);
  m_Animation->start();
  event->accept();
}