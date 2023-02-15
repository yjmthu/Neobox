#include <usbdlg.hpp>
#include <yjson.h>
#include <usbdlgitem.hpp>
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
#include <QSocketNotifier>
// #include <QDebug>

#ifdef _WIN32
#include <windows.h>
#include <dbt.h>
#else
#include <linux/netlink.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <map>
#include <numeric>

// namespace fs = std::filesystem;
#endif

UsbDlg::ItemMap UsbDlg::m_Items;

UsbDlg::UsbDlg(YJson& settings)
  : WidgetBase(nullptr)
  , m_Settings(settings)
  , m_CenterWidget(new QWidget(this))
  , m_MainLayout(new QVBoxLayout(m_CenterWidget))
  , m_Animation(new QPropertyAnimation(this, "geometry"))
  , m_Position(settings[u8"Position"])
  , m_NetlinkSocket(-1)
  , m_SocketNotifier(nullptr)
{
  setWindowFlag(Qt::WindowStaysOnTopHint, m_Settings[u8"StayOnTop"].isTrue());
  setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::Tool);
  setWindowTitle("U盘助手");
  setAttribute(Qt::WA_TranslucentBackground, true);
  SetupUi();
  SetupAnimation();
  GetUsbInfo();

  MessageLoop();

  connect(this, &UsbDlg::UsbAdd, this, &UsbDlg::DoDeviceArrival);
  connect(this, &UsbDlg::UsbRemove, this, &UsbDlg::DoDeviceRemoveComplete);
  // connect(this, &UsbDlg::UsbAdd, this, &UsbDlg::DoDeviceArrival);
}

UsbDlg::~UsbDlg()
{
#ifdef _WIN32
  SHAppBarMessage(ABM_REMOVE, reinterpret_cast<APPBARDATA*>(m_AppBarData));
#else
  if (m_SocketNotifier) {
    m_SocketNotifier->setEnabled(false);
    delete m_SocketNotifier;
  }
  if (m_NetlinkSocket != -1) {
    ::close(m_NetlinkSocket);
  }
#endif
  delete m_CenterWidget;
#ifdef _WIN32
  delete reinterpret_cast<APPBARDATA*>(m_AppBarData);
#endif
}

void UsbDlg::MessageLoop()
{
  // https://blog.csdn.net/qq_40602000/article/details/109553334
  constexpr auto BUF_SIZE = 4096;
  struct sockaddr_nl nls {
    .nl_family = AF_NETLINK,
    .nl_pad = 0,
    .nl_pid = static_cast<__u32>(getpid()),
    .nl_groups = NETLINK_KOBJECT_UEVENT,
  };

  m_NetlinkSocket = ::socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
  if (m_NetlinkSocket == -1) {
    return;
  }

  if (bind(m_NetlinkSocket, (struct sockaddr *)&nls, sizeof(nls)) == -1) {
    return;
  }

  m_SocketNotifier = new QSocketNotifier(m_NetlinkSocket, QSocketNotifier::Read, this);
  connect(m_SocketNotifier, &QSocketNotifier::activated, this, [this](int value){
    std::vector<char> buf(BUF_SIZE, 0);
    // auto const len = recvmsg(sock, &smsg, 0);
    auto const len = read(m_SocketNotifier->socket(), buf.data(), BUF_SIZE*2);

    if (len < 0) {
      std::cout << "error receiving message.\n";
      return;
    }

    if (len > BUF_SIZE - 1) {
      std::cout << "buffer size " << len << " is too small\n";
      return;
    }

    buf[len] = '\0';
    if (strncmp(buf.data(), "libudev", 7)) {
       return;
    }

    std::string_view view(buf.data() + 40, len - 40);

    if (view.find("SUBSYSTEM=block") == std::string_view::npos) {
      return;
    }

    if (view.find("ID_FS_TYPE") == std::string_view::npos) {
      return;
    }

    // std::map<std::string, std::string> infoMap;
    printf("Received %ld bytes\n", len);

    auto action = view.find("ACTION");
    if (action == std::string_view::npos) {
      return;
    }
    action += 7;

    auto devName = view.find("DEVNAME");

    if (devName == std::string_view::npos) {
      return;
    }
    devName += 13;

    if (!strcmp("add", view.data() + action)) {
      emit UsbAdd(QString::fromLocal8Bit(view.data() + devName));
    } else if (!strcmp("change", view.data() + action)) {
      emit UsbChange(QString::fromLocal8Bit(view.data() + devName));
    } else if (!strcmp("remove", view.data() + action)) {
      emit UsbRemove(QString::fromLocal8Bit(view.data() + devName));
    }

    // for(auto i = view.begin(), j = i, k = i; i != view.end(); ++i) {
    //   if (*i == '=') k = i;
    //   if (*i != '\0') continue;
    //   if (k > j) {
    //     infoMap.insert(std::pair {
    //       std::string(j, k),
    //       std::string(k + 1, i)
    //     });
    //   }
    //   j = i + 1;
    // }
    // for (auto& [i, j]: infoMap) {
    //   std::cout << i << ": " << j << std::endl;
    // }
    // std::cout.put('\n');
  }); //will always active
  m_SocketNotifier->setEnabled(true);
}

void UsbDlg::SetupUi()
{
  // setMinimumWidth(300);

  // 主控件设置
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(11, 11, 11, 11);
  layout->addWidget(m_CenterWidget);
  m_CenterWidget->setStyleSheet(
    "QWidget { background-color: white; color: black; border-radius: 3px; }"
    "QPushButton { border-radius: 7px; }"
    "QToolTip { font-size: 9pt; }"
  );
  SetShadowAround(m_CenterWidget, 30);

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
#ifdef _WIN32
  SetHideFullScreen();
#endif
}

void UsbDlg::GetUsbInfo()
{
#ifdef _WIN32
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
#else
  std::ifstream file("/proc/partitions");
  int magor, minor;
  uint64_t blocks;
  std::map<std::string, std::pair<bool, int>> removable;

  for (std::string line, name; std::getline(file, line);) {
    line.erase(0, line.find_first_not_of(' '));
    if (line.empty()) continue;
    if (!std::isdigit(line.front())) continue;
    std::istringstream(line) >> magor >> minor >> blocks >> name;
    if (!std::isdigit(name.back())) {
      std::ifstream f("/sys/class/block/" + name + "/removable");
      removable[name] = { f.get() == '1', 0 };
      f.close();
    } else {
      auto iter = name.crbegin();
      while (std::isdigit(*iter)) --iter;
      auto& data = removable[name.substr(0, name.crend() - iter)];
      data.second += data.first;
    }
  }
  file.close();

  for (auto& [name, data]: removable) {
    if (!data.first) continue;

    for (auto i = data.second ? 1 : 0; i <= data.second; ++i) {
      auto id = name;
      if (i) id += std::to_string(i);
      auto const item = new UsbDlgItem(this, id, m_Items);
      m_MainLayout->addWidget(item);
      item->show();
    }
  }

#endif

  if (!m_Items.empty()) {
    show();
  }
}

#ifdef _WIN32
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
#endif

#ifdef _WIN32
void UsbDlg::DoDeviceArrival(void const* lpdb)
#else
void UsbDlg::DoDeviceArrival(const QString& lpdb)
#endif
{
#ifdef _WIN32
  for (auto c: GetDrives(lpdb)) {
    if (auto iter = m_Items.find(c); iter != m_Items.end())
      continue;
    auto const item = new UsbDlgItem(this, c, m_Items);
    m_MainLayout->addWidget(item);
    item->show();
  }
#else
  // auto const id = std::string("sdb");
  auto bits = lpdb.toLocal8Bit();
  std::string const id(bits.begin(), bits.end());
  if (auto iter = m_Items.find(id); iter != m_Items.end())
    return;
  auto const item = new UsbDlgItem(this, id, m_Items);
  m_MainLayout->addWidget(item);
  item->show();
#endif
  if (m_Items.size() == 1 && !isVisible()) {
    show();
  }
}

#ifdef _WIN32
void UsbDlg::DoDeviceRemoveComplete(const void* lpdb)
#else
void UsbDlg::DoDeviceRemoveComplete(const QString& lpdb)
#endif
{
#ifdef _WIN32
  for (auto c: GetDrives(lpdb)) {
    if (auto iter = m_Items.find(c); iter != m_Items.end()) {
      delete iter->second;
      // if (isVisible()) adjustSize();
    }
  }
#else
  // auto const id = std::string("sdb");
  auto bits = lpdb.toLocal8Bit();
  std::string const id(bits.begin(), bits.end());
  if (auto iter = m_Items.find(id); iter != m_Items.end()) {
    delete iter->second;
    // if (isVisible()) adjustSize();
  }
#endif
  if (m_Items.empty() && isVisible()) {
    hide();
  }
}

#ifdef _WIN32
void UsbDlg::SetHideFullScreen() {
  m_AppBarData = new APPBARDATA {
    sizeof(APPBARDATA),
    reinterpret_cast<HWND>(winId()), 
    static_cast<UINT>(MsgCode::MSG_APPBAR_MSGID),
    0, 0, 0
  };
  SHAppBarMessage(ABM_NEW, reinterpret_cast<APPBARDATA*>(m_AppBarData));
}
#endif

#ifdef _WIN32
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
#endif

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
