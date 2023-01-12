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
#include <QWindow>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

#include <windows.h>
#include <dbt.h>

UsbDlg::ItemMap UsbDlg::m_Items;

UsbDlg::UsbDlg(YJson& settings)
  : WidgetBase(nullptr)
  , m_Settings(settings)
  , m_CenterWidget(new QWidget(this))
  , m_MainLayout(new QVBoxLayout(m_CenterWidget))
  , m_Animation(new QPropertyAnimation(this, "geometry"))
  , m_Position(settings[u8"Position"])
{
  setWindowFlag(Qt::WindowStaysOnTopHint, m_Settings[u8"StayOnTop"].isTrue());
  setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::Tool);
  setWindowTitle("U盘助手");
  setAttribute(Qt::WA_TranslucentBackground, true);
  SetupUi();
  SetupAnimation();
  GetUsbInfo();
}

UsbDlg::~UsbDlg()
{
  SHAppBarMessage(ABM_REMOVE, reinterpret_cast<APPBARDATA*>(m_AppBarData));
  delete m_CenterWidget;
  delete reinterpret_cast<APPBARDATA*>(m_AppBarData);
}

void UsbDlg::SetupUi()
{
  // setMinimumWidth(300);

  // 主控件设置
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(2, 2, 2, 2);
  layout->addWidget(m_CenterWidget);
  m_CenterWidget->setStyleSheet(
    "QWidget { background-color: white; border-radius: 3px; }"
    "QPushButton { border-radius: 7px; }"
    "QToolTip { font-size: 9pt; }"
  );
  auto const effect = new QGraphicsDropShadowEffect(this);
  effect->setOffset(0, 0);
  effect->setColor(Qt::darkGray);
  effect->setBlurRadius(30);
  m_CenterWidget->setGraphicsEffect(effect);

  // 内容设置
  m_MainLayout->setContentsMargins(5, 5, 5, 0);
  m_MainLayout->setSpacing(0);

  auto const titleLayout = new QHBoxLayout;
  auto const label = new QLabel("<span style='font-size:10pt;'>U盘助手</span>", this);
  auto const btnTop = new QPushButton(this);
  auto const btnClose = new QPushButton(this);

  layout->setSizeConstraint(QLayout::SetFixedSize);    // 自动调节大小
  titleLayout->setContentsMargins(5, 0, 10, 0);
  m_MainLayout->addLayout(titleLayout);
  titleLayout->setSpacing(13);
  titleLayout->addWidget(label);
  titleLayout->addStretch();
  titleLayout->addWidget(btnTop);
  titleLayout->addWidget(btnClose);

  auto const sizeBtn = QSize(14, 14);
  btnTop->setCheckable(true);
  btnTop->setChecked(m_Settings[u8"StayOnTop"].isTrue());
  btnTop->setFixedSize(sizeBtn);
  btnTop->setStyleSheet(
    "QPushButton {"
      "background-color: gray;"
    "}"
    "QPushButton:hover {"
      "background-color: #d35f5f;"
      "border-image: url(:/icons/usb-top.png);"
    "}"
    "QPushButton:checked {"
      "background-color: #ff9955;"
    "}"
    "QPushButton:checked:hover {"
      "background-color: #ff9955;"
      "border-image: url(:/icons/usb-top.png);"
    "}"
  );
  btnTop->setToolTip("置顶");

  btnClose->setFixedSize(sizeBtn);
  btnClose->setStyleSheet(
    "QPushButton {"
      "background-color: #8dd35f;"
    "}"
    "QPushButton:hover {"
      "border-image: url(:/icons/usb-hide.png);"
      "background-color: #8dd35f;"
    "}"
  );
  btnClose->setToolTip("隐藏");

  // 信号设置
  connect(btnClose, &QPushButton::clicked, this, &QWidget::hide);
  connect(btnTop, &QPushButton::clicked, this, [this](bool on) {
    // https://blog.csdn.net/wangw8507/article/details/116912796
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
  wchar_t szDevicePath[] = {
    L'\\', L'\\', L'.', L'\\',
    L'*', L':', L'\0'
  };        
  wchar_t* szDiskPath = szDevicePath + 4;  // 有效节约空间
		
	wchar_t driver = L'A';
	DWORD dwBytesReturned = 0;
	// STORAGE_DEVICE_NUMBER deviceNumber;

  STORAGE_DEVICE_DESCRIPTOR deviceDescriptor;
  STORAGE_PROPERTY_QUERY   propertyQuery;

  // ++i 放在前面，防止 continue 跳过
	for (auto disksMask = GetLogicalDrives(); disksMask; (disksMask >>= 1), ++driver)
  {
		if (!(disksMask & 1)) continue;

    *szDiskPath = driver;
    auto const uDriveType = GetDriveTypeW(szDiskPath);

    // 光盘为 DRIVE_CDROM
    
    if (uDriveType != DRIVE_REMOVABLE) {
      if (uDriveType != DRIVE_FIXED) continue;

      // 移动硬盘检测
      auto const hDevice = CreateFileW(szDevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hDevice == INVALID_HANDLE_VALUE) continue;

      // https://blog.csdn.net/slfkj/article/details/90437411
      BOOL bGetOk = FALSE;
      propertyQuery.QueryType = PropertyStandardQuery;
      propertyQuery.PropertyId = StorageDeviceProperty;
      bGetOk = DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &propertyQuery, sizeof(propertyQuery),
          &deviceDescriptor, sizeof(deviceDescriptor), &dwBytesReturned, NULL);
      CloseHandle(hDevice);

      if (!bGetOk || deviceDescriptor.BusType != BusTypeUsb) continue;
      
    }

    auto const item = new UsbDlgItem(this, driver, m_Items);
    m_MainLayout->addWidget(item);
    item->show();
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
  if (bHide.isFalse()) return false;
  
  MSG* msg = static_cast<MSG*>(message);
  if (MsgCode::MSG_APPBAR_MSGID != static_cast<MsgCode>(msg->message)) return false;
  switch ((UINT)msg->wParam) {
    case ABN_FULLSCREENAPP:
      if (msg->lParam) {
        hide();
      } else if (!m_Items.empty()) {
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

void UsbDlg::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    m_Position = YJson::A{x(), y()};
    mgr->SaveSettings();
  }
  this->WidgetBase::mouseReleaseEvent(event);
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