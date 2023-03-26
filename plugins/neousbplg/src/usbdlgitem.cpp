#include <usbdlgitem.hpp>
#include <pluginmgr.h>

#ifdef _WIN32
#include <Windows.h>
#include <dbt.h>
#include <cfg.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#else
#include <iostream>
#include <fstream>
#include <filesystem>
#include <mntent.h>
#include <sys/vfs.h>
// #include <sys/mount.h>
#include <unistd.h>
#include <pwd.h>
#include <wait.h>

namespace fs = std::filesystem;
using namespace std::literals;
#endif

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QFrame>
#include <QLabel>
#include <QEvent>

#include <format>

#ifdef _WIN32
UsbDlgItem::UsbDlgItem(QWidget* parent, char id, UsbDlg::ItemMap& map)
#else
UsbDlgItem::UsbDlgItem(QWidget* parent, std::string id, UsbDlg::ItemMap& map)
#endif
  : QWidget(parent)
  , m_Items(map)
#ifdef _WIN32
  , m_DriveId(id)
  , m_DrivePath(new wchar_t[] { m_DriveId, L':', L'\\', L'\0' })
#else
  , m_DriveId(id)
#endif
  , m_UsbSizeLogo(new QWidget(this))
  , m_UsbSizeLogoColorMask(new QFrame(m_UsbSizeLogo))
  , m_UsbInfoText(new QLabel(this))
  , m_BtnOpen(new QToolButton(this))
  , m_BtnEject(new QToolButton(this))
{
  m_Items[id] = this;
  UpdateUsbName();
  UpdateUsbSize();
  SetupUi();
  SetStyleSheet();
#ifdef __linux__
  QObject::connect(qobject_cast<UsbDlg*>(parent), &UsbDlg::UsbChange, this, &UsbDlgItem::DoUsbChange);
#endif
}

UsbDlgItem::~UsbDlgItem()
{
  m_Items.erase(m_DriveId);
#ifdef _WIN32
  delete [] m_DrivePath;
#endif
}

#ifdef __linux__
void UsbDlgItem::DoUsbChange(QString id)
{
  auto const name = id.toLocal8Bit();
  if (!std::equal(name.begin(), name.end(), m_DriveId.begin(), m_DriveId.end())) {
    return;
  }
  UpdateUsbSize();
}
#endif

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
#ifdef _WIN32
  m_BtnOpen->setText("打开");
#endif
  m_BtnOpen->setFixedWidth(45);
  m_BtnOpen->setIcon(QIcon(":/icons/usb-open.png"));
  m_BtnOpen->setIconSize(QSize(25, 25));
  m_BtnOpen->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  m_BtnEject->setText("弹出");
  m_BtnEject->setFixedWidth(45);
  m_BtnEject->setIcon(QIcon(":/icons/usb-eject.png"));
  m_BtnEject->setIconSize(QSize(25, 25));
  m_BtnEject->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  mainLayout->addWidget(m_UsbSizeLogo);
  mainLayout->addWidget(m_UsbInfoText);
  mainLayout->addWidget(m_BtnOpen);
  mainLayout->addWidget(m_BtnEject);

  m_UsbSizeLogo->installEventFilter(this);
  connect(m_BtnOpen, &QToolButton::clicked, this, [this](){
#ifdef _WIN32
    ShellExecuteW(nullptr, L"open", L"explorer", m_DrivePath, nullptr, SW_SHOWNORMAL);
#else
    // std::cout << m_MountPoint << std::endl;
    if (fs::exists(m_MountPoint)) {
      QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(m_MountPoint)));
    } else {
      mgr->ShowMsg(MountUsb() ? "挂载成功": "挂载失败");
    }
#endif
  });
  connect(m_BtnEject, &QToolButton::clicked, this, std::bind(&UsbDlgItem::PopUsbDrive, this));
}

bool UsbDlgItem::IsDiskExist() const
{
#ifdef _WIN32
  return GetLogicalDrives() & (1 << (m_DriveId - 'A'));
#else
  return fs::exists("/dev/" + m_DriveId);
#endif
}

void UsbDlgItem::SetUsbInfoText()
{
  auto const str = std::format(
#ifdef _WIN32
    L""
#endif
    "<p>{} : {} </p><p>{}/{} </p>",
    m_DriveId, m_DriveName,
    FormatSize(m_SizeFree),
    FormatSize(m_SizeTotal)
  );
#ifdef _WIN32
  auto text = QString::fromStdWString(str);
#else
  auto text = QString::fromStdString(str);
#endif
  m_UsbInfoText->setText(text);
  m_UsbInfoText->setToolTip(text.mid(3, text.lastIndexOf("<p>") - 4));
}

#ifdef __linux__
static std::string GetMountPoint(const char* path) {
  std::string result;
  auto const mntfile = setmntent("/proc/mounts", "r");
  if (!mntfile) {
    return result;
  }

  for (mntent *mntent = nullptr; (mntent = getmntent(mntfile));) {
    if (strcmp(mntent->mnt_fsname, path) == 0) {
      result = mntent->mnt_dir;
      // printf("%s, %s, %s, %s\n",
      //   mntent->mnt_dir,
      //   mntent->mnt_fsname,
      //   mntent->mnt_type,
      //   mntent->mnt_opts);
      break;
    }
  }
  endmntent(mntfile);

  return result;
}

#endif

bool UsbDlgItem::UpdateUsbSize()
{
#ifdef _WIN32
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
#else
  if (IsMounted()) {
    m_BtnOpen->setText("打开");
    struct statfs diskInfo;
    if (statfs(m_MountPoint.c_str(), &diskInfo) < 0) {
      return false;
    }
    const size_t totalBlocks = diskInfo.f_bsize;  
    m_SizeTotal = totalBlocks * diskInfo.f_blocks;  
    m_SizeFree = diskInfo.f_bfree * totalBlocks;  
  } else if (UpdateMountPoint()) {
    m_BtnOpen->setText("挂载");
    std::ifstream file("/sys/class/block/" + m_DriveId + "/size");
    if (!file.is_open()) return false;
    file >> m_SizeTotal;
    file.close();
    m_SizeTotal <<= 9;
    m_SizeFree = m_SizeTotal;
  }
#endif

  return true;
}

static std::optional<char> Hex2Char(const char* str)
{
  std::string c = { '0', str[1], str[2], str[3], '\0' };
  try {
    return std::stoi(c, nullptr, 16);
  } catch (...) {
    return std::nullopt;
  }
}

bool UsbDlgItem::UpdateUsbName()
{
#ifdef _WIN32
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
#else
  for (auto& dirEntrey: fs::directory_iterator("/dev/disk/by-label")) {
    if (!dirEntrey.is_symlink()) continue;

    auto path = fs::read_symlink(dirEntrey);
    if (path.stem().string() == m_DriveId) {
      auto buffer = dirEntrey.path().stem().string();
      m_DriveName.clear();
      for (auto iter=buffer.begin(); iter != buffer.end(); ++iter) {
        if (*iter == '\\' && iter + 3 < buffer.end()) {
          auto opt = Hex2Char(iter.base());
          if (opt) {
            m_DriveName.push_back(*opt);
          } else {
            m_DriveName.push_back(' ');
          }
          iter += 3;
        } else {
          m_DriveName.push_back(*iter);
        }
      }
      break;
    }
  }
  return false;
#endif
}

#ifdef __linux__
bool UsbDlgItem::UpdateMountPoint()
{
  auto const pwd = getpwuid(getuid());
  if (!pwd) return false;

  m_MountPoint = "/run/media/"s + pwd->pw_name + "/"s + m_DriveName;
  return true;
}

bool UsbDlgItem::IsMounted()
{
  auto path = "/dev/" + m_DriveId;
  m_MountPoint = GetMountPoint(path.c_str());
  return !m_MountPoint.empty();
}

bool UsbDlgItem::MountUsb()
{
  if (IsMounted()) {
    m_BtnOpen->setText("打开");
    return true;
  }
  if (UpdateMountPoint()) {
    auto const path = "/dev/" + m_DriveId;
    int chRetrun;
    pid_t pid = fork();
  
    if (pid == 0){
      execlp("udisksctl", "udisksctl", "mount", "-b", path.c_str(), nullptr);
    } else if (pid > 0) {
      wait(&chRetrun);
      // printf("wait process's pid=%d,status=0x%X,exit value=%d(0x%X)\n", pid2, child_ret,
      //     (child_ret), WEXITSTATUS(child_ret));
      if (WIFEXITED(chRetrun)) {
        m_BtnOpen->setText("打开");
        return true;
      }
    } else {
      mgr->ShowMsg("该功能需要安装 udisks2");
    }

  }
  m_BtnOpen->setText("挂载");
  return false;
}

bool UsbDlgItem::UmountUsb()
{
  if (IsMounted()) {
    // auto const path = "/dev/" + m_DriveId;
    // if (umount(path.c_str()) < 0) {
    //   m_BtnOpen->setText("打开");
    //   return false;
    // }
    auto const path = "/dev/" + m_DriveId;
    int chRetrun;
    pid_t pid = fork();
  
    if (pid == 0){
      execlp("udisksctl", "udisksctl", "unmount", "-b", path.c_str(), nullptr);
    } else if (pid > 0) {
      wait(&chRetrun);
      // printf("wait process's pid=%d,status=0x%X,exit value=%d(0x%X)\n", pid2, child_ret,
      //     (child_ret), WEXITSTATUS(child_ret));
      if (WIFEXITED(chRetrun) == 0) {
        m_BtnOpen->setText("打开");
        return false;
      }
    } else {
      mgr->ShowMsg("该功能需要安装 udisks2");
    }
  }
  m_BtnOpen->setText("挂载");
  return true;
}
#endif

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

UsbDlgItem::String UsbDlgItem::FormatSize(uint64_t size)
{
  const std::array<String, 6> uints { 
#ifdef _WIN32
    L"B", L"KB", L"MB", L"GB", L"TB", L"PB"
#else
    "B", "KB", "MB", "GB", "TB", "PB"
#endif
  };
  auto iter = uints.begin();
  uint64_t base = 1;

  size &= (((uint64_t)1 << 60) - 1);

  for (auto const kb = (size >> 10); base <= kb; ++iter)
  {
    base <<= 10;
  }

  return std::format(
#ifdef _WIN32
    L""
#endif
    "{:.1f} {}",
    static_cast<double>(size) / base, *iter);
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
#ifdef __linux__
    } else {
      auto const path = "/dev/" + m_DriveId;
      int chRetrun;
      pid_t pid = fork();
    
      if (pid == 0){
        execlp("udisksctl", "udisksctl", "power-off", "-b", path.c_str(), nullptr);
      } else if (pid > 0) {
        wait(&chRetrun);
        // printf("wait process's pid=%d,status=0x%X,exit value=%d(0x%X)\n", pid2, child_ret,
        //     (child_ret), WEXITSTATUS(child_ret));
        if (WIFEXITED(chRetrun) == 0) {
          m_Items[m_DriveId] = this;
          mgr->ShowMsg("弹出失败");
          return;
        }
      } else {
        mgr->ShowMsg("该功能需要安装 udisks2");
      }
#endif
    }
  }
  mgr->ShowMsg("弹出成功");

  deleteLater();
}

#ifdef _WIN32
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
#endif

bool UsbDlgItem::EjectUsbDisk()
{
#ifdef _WIN32
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
#else
  bool res = false;
  std::ifstream busy("/sys/class/block/" + m_DriveId + "/device/device_busy");
  res = busy.get() == '0';
  busy.close();

  if (!res) return false;

  return UmountUsb();
#endif
}
