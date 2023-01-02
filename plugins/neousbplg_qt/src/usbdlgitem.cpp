#include <usbdlgitem.h>
#include <neoapp.h>

#include <Windows.h>
#include <dbt.h>
#include <cfg.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QLabel>

#include <format>

UsbDlgItem::UsbDlgItem(QWidget* parent, char id, UsbDlg::ItemMap& map)
  : QWidget(parent)
  , m_Items(map)
  , m_DriveId(id)
  , m_DrivePath(new wchar_t[] { m_DriveId, L':', L'\\', L'\0' })
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

void UsbDlgItem::SetupUi()
{
  auto const size = QSize(40, 40);
  auto const mainLayout = new QHBoxLayout(this);
  auto const usbSizeLogo = new QFrame(this);
  usbSizeLogo->setMinimumSize(size);
  usbSizeLogo->setMaximumSize(size);
  usbSizeLogo->setStyleSheet("background-color: #ffa924; border-radius: 20px;");
  auto const usbSizeLogoColorMask = new QFrame(usbSizeLogo);
  usbSizeLogoColorMask->setGeometry(0, 0, size.width(), size.height());
  usbSizeLogoColorMask->setStyleSheet(GetStyleSheet());
  auto const usbSizeLogoImageMask = new QFrame(usbSizeLogo);
  usbSizeLogoImageMask->setGeometry(3, 3, size.width() - 6, size.height() - 6);
  usbSizeLogoImageMask->setStyleSheet("background-color: white; border-radius: 17px; border-image:url(:/icons/usblogo.png);");
  auto const usbInfoText = new QLabel(GetUsbInfoText(), this);
  auto const btnOpen = new QPushButton("打开", this);
  auto const btnPop = new QPushButton("弹出", this);
  mainLayout->addWidget(usbSizeLogo);
  mainLayout->addWidget(usbInfoText);
  mainLayout->addWidget(btnOpen);
  mainLayout->addWidget(btnPop);

  connect(btnOpen, &QPushButton::clicked, this, std::bind(ShellExecuteW, nullptr, L"open", L"explorer", m_DrivePath, nullptr, SW_SHOWNORMAL));
  connect(btnPop, &QPushButton::clicked, this, std::bind(&UsbDlgItem::PopUsbDrive, this));
}

bool UsbDlgItem::IsDiskExist() const
{
  return GetLogicalDrives() & (1 << (m_DriveId - 'A'));
}

QString UsbDlgItem::GetUsbInfoText() const
{
  auto const str = std::format(
    L"<p>{}: {}</p>"
    "<p>{}/{}</p>",
    m_DriveId, m_DriveName,
    FormatSize(m_SizeFree),
    FormatSize(m_SizeTotal)
  );
  return QString::fromStdWString(str);
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
  setStyleSheet(
    "QPushButton {"
      "background-color: rgba(200, 200, 200, 100);"
    "}"
    "QPushButton:hover {"
      "background-color: rgba(150, 150, 150, 100);"
    "}"
    "QPushButton:pressed {"
      "background-color: rgba(100, 100, 100, 100);"
    "}"
  );
}

std::wstring UsbDlgItem::FormatSize(uint64_t size)
{
  const std::array<std::wstring, 6> uints { L"B", L"KB", L"MB", L"GB", L"TB", L"PB" };
  auto iter = uints.begin();
  uint64_t base = 1;

  size &= (((uint64_t)1 << 60) - 1);

  for (auto const kb = (size >> 10); base < kb; ++iter)
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
      glb->glbShowMsg("弹出失败");
      return;
    }
  }
  glb->glbShowMsg("弹出成功");

  deleteLater();
}

int UsbDlgItem::GetDrivesDevInstByDiskNumber(const unsigned long diskNumber)
{
  // https://www.marxcbr.cn/archives/7fc2b650

  const auto h_dev_info = SetupDiGetClassDevs(&GUID_DEVINTERFACE_DISK, NULL, NULL,
                                            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

  if (h_dev_info == INVALID_HANDLE_VALUE)
      return 0;
  DWORD dw_index = 0;
  SP_DEVICE_INTERFACE_DATA dev_interface_data = { 0 };
  dev_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);


  SP_DEVICE_INTERFACE_DATA sp_did;
  SP_DEVINFO_DATA sp_dd;
  DWORD dw_size;

  sp_did.cbSize = sizeof(sp_did);

  while (SetupDiEnumDeviceInterfaces(h_dev_info, NULL, &GUID_DEVINTERFACE_DISK, dw_index,
                                        &dev_interface_data)) {

    SetupDiEnumInterfaceDevice(h_dev_info, NULL, &GUID_DEVINTERFACE_DISK, dw_index, &sp_did);

    dw_size = 0;
    SetupDiGetDeviceInterfaceDetail(h_dev_info, &sp_did, NULL, 0, &dw_size, NULL);

    if (dw_size) {
      auto psp_did = static_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(HeapAlloc(
                          GetProcessHeap(), HEAP_ZERO_MEMORY, dw_size));
      if (psp_did == NULL) {
          continue; // autsch
      }
      psp_did->cbSize = sizeof(*psp_did);
      ZeroMemory(static_cast<PVOID>(&sp_dd), sizeof(sp_dd));
      sp_dd.cbSize = sizeof(sp_dd);


      auto res = SetupDiGetDeviceInterfaceDetailW(h_dev_info, &sp_did, psp_did, dw_size, &dw_size, &sp_dd);
      if (res) {
        const auto h_drive = CreateFileW(psp_did->DevicePath, 0,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
        if (h_drive != INVALID_HANDLE_VALUE) {
            STORAGE_DEVICE_NUMBER sdn;
          DWORD dw_bytes_returned = 0;
          res = DeviceIoControl(h_drive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dw_bytes_returned, NULL);
          if (res) {
            if (diskNumber == static_cast<long>(sdn.DeviceNumber)) {
              CloseHandle(h_drive);
              SetupDiDestroyDeviceInfoList(h_dev_info);
              return sp_dd.DevInst;
            }
          }
          CloseHandle(h_drive);
        }
      }
      HeapFree(GetProcessHeap(), 0, psp_did);
    }
    dw_index++;
  }

  SetupDiDestroyDeviceInfoList(h_dev_info);

  return 0;
}


int UsbDlgItem::EjectUsbDisk()
{
  // https://forums.codeguru.com/showthread.php?499595-RESOLVED-Need-to-get-name-of-a-USB-flash-drive-device
  const wchar_t szVolumePath[] { L'\\', L'\\', L'.', L'\\', m_DriveId, L':', L'\0' };
  HANDLE hVolume = CreateFileW(szVolumePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
  DWORD dwBytesReturned = 0;
  if (hVolume == INVALID_HANDLE_VALUE) {
    glb->glbShowMsg("获取磁盘标识失败！");
    return 0;
  }

  STORAGE_DEVICE_NUMBER sdn;
  if (!DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL))
  {
    glb->glbShowMsg("获取磁盘序号失败！");
    CloseHandle(hVolume);
    return 0;
  }
  CloseHandle(hVolume);

  unsigned long dev_inst = GetDrivesDevInstByDiskNumber(sdn.DeviceNumber);
  if (dev_inst == 0) {
      glb->glbShowMsg("GetDrivesDevInstDiskNumber failed！");
      return 0;
  }

  ULONG status = 0;
  ULONG problem_number = 0;
  auto veto_type = PNP_VetoTypeUnknown;
  wchar_t veto_name[MAX_PATH];

  auto res = CM_Get_Parent(&dev_inst, dev_inst, 0);

	if (res != CR_SUCCESS)
	{
		return 0;
	}
	res = CM_Get_DevNode_Status(&status, &problem_number, dev_inst, 0);
	if (res != CR_SUCCESS)
	{
		return 0;
	}
  const auto is_removable = ((status & DN_REMOVABLE) != 0);

  if (!is_removable)
    glb->glbShowMsg("磁盘正在使用中！");
  
  veto_name[0] = '\0';
  if (is_removable)
    res = CM_Request_Device_EjectW(dev_inst, &veto_type, veto_name, MAX_PATH, 0);
  else
    res = CM_Query_And_Remove_SubTreeW(dev_inst, &veto_type, veto_name, MAX_PATH, 0);
  return (res == CR_SUCCESS && veto_name[0] == '\0');
}
