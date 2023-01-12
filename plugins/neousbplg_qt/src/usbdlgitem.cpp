#include <usbdlgitem.h>
#include <pluginmgr.h>

#include <Windows.h>
#include <dbt.h>
#include <cfg.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QFrame>
#include <QLabel>
#include <QEvent>

#include <format>

UsbDlgItem::UsbDlgItem(QWidget* parent, char id, UsbDlg::ItemMap& map)
  : QWidget(parent)
  , m_Items(map)
  , m_DriveId(id)
  , m_DrivePath(new wchar_t[] { m_DriveId, L':', L'\\', L'\0' })
  , m_UsbSizeLogo(new QWidget(this))
  , m_UsbSizeLogoColorMask(new QFrame(m_UsbSizeLogo))
  , m_UsbInfoText(new QLabel(this))

{
  m_Items[id] = this;
  UpdateUsbSize();
  UpdateUsbName();
  SetupUi();
  SetStyleSheet();
}

UsbDlgItem::~UsbDlgItem()
{
  m_Items.erase(m_DriveId);
  delete [] m_DrivePath;
}

bool UsbDlgItem::eventFilter(QObject* target, QEvent* event)
{
  // static bool bShiftDown = false, bCtrlDown = false;
  if (target == m_UsbSizeLogo) {
    switch (event->type()) { 
    case QEvent::MouseButtonDblClick:
      UpdateUsbSize();
      UpdateUsbName();
      SetUsbInfoText();
      m_UsbSizeLogoColorMask->setStyleSheet(GetStyleSheet());
      mgr->ShowMsg("刷新成功！");
      return true;
    default:
      break;
    }
  }
  return false;
}

void UsbDlgItem::SetupUi()
{
  auto const size = QSize(40, 40);
  auto const mainLayout = new QHBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  m_UsbSizeLogo->setFixedSize(size);
  m_UsbSizeLogo->setStyleSheet(  // ffa924
    "background-color: #8dd35f;"
      // "qradialgradient("
      //   "spread:pad, cx:0.5, cy:0.5, "
      //   "radius:0.5, fx:0.5, fy:0.5, "
      //   "stop:0 #8dd35f, "
      //   "stop:0.98400 #8dd35f, "
      //   "stop:0.98401 transparent, "
      //   "stop:1 transparent"
      // ");"
    "border-radius: 20px;"
  );
  m_UsbSizeLogoColorMask->setGeometry(0, 0, size.width(), size.height());
  m_UsbSizeLogoColorMask->setStyleSheet(GetStyleSheet());

  auto const usbSizeLogoImageMask = new QFrame(m_UsbSizeLogo);
  usbSizeLogoImageMask->setGeometry(3, 3, size.width() - 6, size.height() - 6);
  usbSizeLogoImageMask->setStyleSheet(
    "background-color: white;"
    "border-radius: 17px;"
    "border-image:url(:/icons/usblogo.png);"
  );
  SetUsbInfoText();

  m_UsbInfoText->setFixedWidth(110);
  auto const btnOpen = new QToolButton(this);
  btnOpen->setText("打开");
  btnOpen->setFixedWidth(45);
  btnOpen->setIcon(QIcon(":/icons/usb-open.png"));
  btnOpen->setIconSize(QSize(25, 25));
  btnOpen->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  auto const btnPop = new QToolButton(this);
  btnPop->setText("弹出");
  btnPop->setFixedWidth(45);
  btnPop->setIcon(QIcon(":/icons/usb-eject.png"));
  btnPop->setIconSize(QSize(25, 25));
  btnPop->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  mainLayout->addWidget(m_UsbSizeLogo);
  mainLayout->addWidget(m_UsbInfoText);
  mainLayout->addWidget(btnOpen);
  mainLayout->addWidget(btnPop);

  m_UsbSizeLogo->installEventFilter(this);
  connect(btnOpen, &QToolButton::clicked, this, std::bind(ShellExecuteW, nullptr, L"open", L"explorer", m_DrivePath, nullptr, SW_SHOWNORMAL));
  connect(btnPop, &QToolButton::clicked, this, std::bind(&UsbDlgItem::PopUsbDrive, this));
}

bool UsbDlgItem::IsDiskExist() const
{
  return GetLogicalDrives() & (1 << (m_DriveId - 'A'));
}

void UsbDlgItem::SetUsbInfoText()
{
  auto const str = std::format(
    L"<p>{} : {}</p><p>{}/{}</p>",
    m_DriveId, m_DriveName,
    FormatSize(m_SizeFree),
    FormatSize(m_SizeTotal)
  );
  auto text = QString::fromStdWString(str);
  m_UsbInfoText->setText(text);
  m_UsbInfoText->setToolTip(text.mid(3, text.lastIndexOf("<p>") - 4));
}

bool UsbDlgItem::UpdateUsbSize()
{
  //得出磁盘的可用空间
  DWORD dwTotalClusters;   //总的簇
  DWORD dwFreeClusters;    //可用的簇
  DWORD dwSectPerClust;    //每个簇有多少个扇区
  DWORD dwBytesPerSect;    //每个扇区有多少个字节
  // std::wstring divePath = {
  //   m_DriveId, L':', L'\\', L'\0'
  // };
  BOOL bResult = GetDiskFreeSpaceW(
    m_DrivePath,
    &dwSectPerClust, &dwBytesPerSect,
    &dwFreeClusters, &dwTotalClusters
  );
  if (!bResult) {
    m_SizeTotal = m_SizeFree = 0;
    return false;
  }

  m_SizeTotal = dwTotalClusters * (uint64_t)dwSectPerClust * dwBytesPerSect;

  m_SizeFree = dwFreeClusters * (uint64_t)dwSectPerClust * dwBytesPerSect;

  return true;
}

bool UsbDlgItem::UpdateUsbName()
{
  // https://www.cnblogs.com/james1207/p/3423990.html
  m_DriveName = std::wstring(MAX_PATH + 1, 0);
  BOOL bSucceeded = GetVolumeInformationW(
    m_DrivePath, m_DriveName.data(), MAX_PATH + 1, NULL, NULL, NULL, NULL, 0);
  if (bSucceeded) {
    m_DriveName.erase(m_DriveName.find(L'\0'));
  } else {
    m_DriveName = L"存储设备";
  }
  return bSucceeded;
}

QString UsbDlgItem::GetStyleSheet() const
{
  double constexpr delta = 0.0002;
  auto free = static_cast<double>(m_SizeFree) / m_SizeTotal;
  if (free > delta) free -= delta;
  auto const style = std::format(
    "background-color:"
      "qconicalgradient("
        "cx:0.5, cy:0.5, angle:90, "
        "stop:0 #eeeeee, "
        "stop:{:.4f} #eeeeee, "
        "stop:{:.4f} transparent, "
        "stop:1 transparent"
      ");"
    "border-radius: 20px;",
    free,
    free + delta
  );

  return QString::fromStdString(style);
}

void UsbDlgItem::SetStyleSheet()
{
  // https://stackoverflow.com/questions/1418578/qt-qpushbutton-icon-above-text
  setStyleSheet(
    "QToolButton {"
      "border-radius: 7px;"
      // "background-color: rgba(200, 200, 200, 100);"
      "background-color: transparent;"
      "padding: 2px 8px;"
    "}"
    "QToolButton:hover {"
      "border-radius: 7px;"
      "background-color: rgba(200, 200, 200, 100);"
      "padding: 2px 8px;"
    "}"
    "QToolButton:pressed {"
      "border-radius: 7px;"
      "background-color: rgba(150, 150, 150, 100);"
      "padding: 2px 8px;"
    "}"
    "QLabel {"
      "font-size: 8pt;"
      "padding-top: 15px;"
      "padding-bottom: 15px;"
    "}"
  );
}

std::wstring UsbDlgItem::FormatSize(uint64_t size)
{
  const std::array<std::wstring, 6> uints { L"B", L"KB", L"MB", L"GB", L"TB", L"PB" };
  auto iter = uints.begin();
  uint64_t base = 1;

  size &= (((uint64_t)1 << 60) - 1);

  for (auto const kb = (size >> 10); base <= kb; ++iter)
  {
    base <<= 10;
  }

  return std::format(
    L"{:.1f} {}", static_cast<double>(size) / base, *iter);
}

void UsbDlgItem::PopUsbDrive()
{
  // 先移除指针，以免对话框再次检测到拔出U盘后，出现内存泄漏问题
  m_Items.erase(m_DriveId);

  if (IsDiskExist()) {
    // 检测U盘是否占用，弹出U盘
    if (!EjectUsbDisk()) {
      m_Items[m_DriveId] = this;
      mgr->ShowMsg("弹出失败");
      return;
    }
  }
  mgr->ShowMsg("弹出成功");

  deleteLater();
}

DWORD UsbDlgItem::GetDrivesDevInstByDiskNumber(const DWORD diskNumber)
{
  // https://www.marxcbr.cn/archives/7fc2b650

  const auto hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_DISK, NULL, NULL,
                                            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

  if (hDevInfo == INVALID_HANDLE_VALUE)
    return 0;
  
  const std::unique_ptr<void, BOOL(*)(HDEVINFO)> ptrDevInfo(hDevInfo, &SetupDiDestroyDeviceInfoList);

  SP_DEVICE_INTERFACE_DATA deviceData { 0 };
  SP_DEVINFO_DATA deviceInfo;

  deviceData.cbSize = sizeof(deviceData);

  for (DWORD dwIndex = 0, dwSize; 
      SetupDiEnumInterfaceDevice(ptrDevInfo.get(), NULL, &GUID_DEVINTERFACE_DISK, dwIndex, &deviceData);
      dwIndex++)
  {
    dwSize = 0;
    SetupDiGetDeviceInterfaceDetailW(ptrDevInfo.get(), &deviceData, NULL, 0, &dwSize, NULL);
    if (!dwSize) continue;

    std::unique_ptr<char8_t[]> deviceDetailBuffer(new char8_t[dwSize] { 0 });
    auto deviceDetail = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(deviceDetailBuffer.get());
    deviceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
    deviceInfo.cbSize = sizeof(deviceInfo);

    auto res = SetupDiGetDeviceInterfaceDetailW(ptrDevInfo.get(), &deviceData, deviceDetail, dwSize, &dwSize, &deviceInfo);
    if (!res) continue;

    const auto hDrive = CreateFileW(deviceDetail->DevicePath, 0,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) continue;

    STORAGE_DEVICE_NUMBER sdn;
    dwSize = 0;
    res = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwSize, NULL);
    CloseHandle(hDrive);

    if (res && diskNumber == sdn.DeviceNumber) {
      return deviceInfo.DevInst;
    }
  }
  return 0;
}


bool UsbDlgItem::EjectUsbDisk()
{
  // https://forums.codeguru.com/showthread.php?499595-RESOLVED-Need-to-get-name-of-a-USB-flash-drive-device
  const wchar_t szVolumePath[] { L'\\', L'\\', L'.', L'\\', m_DriveId, L':', L'\0' };
  HANDLE hVolume = CreateFileW(szVolumePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
  DWORD dwBytesReturned = 0;
  if (hVolume == INVALID_HANDLE_VALUE) {
    mgr->ShowMsg("获取磁盘标识失败！");
    return 0;
  }

  STORAGE_DEVICE_NUMBER sdn;
  if (!DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL))
  {
    mgr->ShowMsg("获取磁盘序号失败！");
    CloseHandle(hVolume);
    return 0;
  }
  CloseHandle(hVolume);

  DWORD devInst = GetDrivesDevInstByDiskNumber(sdn.DeviceNumber);
  if (devInst == 0) {
    mgr->ShowMsg("GetDrivesDevInstDiskNumber failed！");
    return 0;
  }

  ULONG status = 0;
  // wchar_t pszVetoName[MAX_PATH] { 0 };

  auto res = CM_Get_Parent(&devInst, devInst, 0);

	if (res == CR_SUCCESS)
	{
    ULONG problemNumber = 0;
		res = CM_Get_DevNode_Status(&status, &problemNumber, devInst, 0);
	}
	if (res == CR_SUCCESS)
	{
    auto vetoType = PNP_VetoTypeUnknown;
    if (status & DN_REMOVABLE) {
      res = CM_Request_Device_EjectW(devInst, &vetoType, NULL, MAX_PATH, 0);
    } else {
      res = CM_Query_And_Remove_SubTreeW(devInst, &vetoType, NULL, MAX_PATH, 0);
      mgr->ShowMsg("磁盘正在使用中！");
    }
  }

  return res == CR_SUCCESS;
}
