#ifndef USBDLGITEM_HPP
#define USBDLGITEM_HPP

#include <QWidget>
#include <map>

#ifdef _WIN32
#include <windows.h>
#else
#endif

#include <usbdlg.hpp>

class UsbDlgItem: public QWidget
{
  Q_OBJECT
#ifdef _WIN32
  typedef wchar_t Char;
#else
  typedef char Char;
#endif
  typedef std::basic_string<Char> String;
protected:
  bool eventFilter(QObject*, QEvent*) override;
public:
#ifdef _WIN32
  explicit UsbDlgItem(QWidget* parent, char id, UsbDlg::ItemMap& map);
#else
  explicit UsbDlgItem(QWidget* parent, std::string id, UsbDlg::ItemMap& map);
#endif
  ~UsbDlgItem();
public:
  void PopUsbDrive();
public slots:
  void DoUsbChange(QString id);
private:
  void SetupUi();
  void SetUsbInfoText();
  bool UpdateUsbSize();
  bool UpdateUsbName();
  QString GetStyleSheet() const;
  void SetStyleSheet();
  static String FormatSize(uint64_t size);
  bool EjectUsbDisk();
  bool IsDiskExist() const;
#ifdef _WIN32
  DWORD GetDrivesDevInstByDiskNumber(const DWORD diskNumber);
#else
  bool UpdateMountPoint();
  bool IsMounted();
  bool MountUsb();
  bool UmountUsb();
#endif

private:
  UsbDlg::ItemMap& m_Items;
#ifdef _WIN32
  const Char m_DriveId;
  const Char* const m_DrivePath;
#else
  String m_MountPoint;
  const String m_DriveId;
#endif
  String m_DriveName;

  uint64_t m_SizeTotal;
  uint64_t m_SizeFree;
  QWidget* const m_UsbSizeLogo;
  class QFrame* const m_UsbSizeLogoColorMask;
  class QLabel* const m_UsbInfoText;
  class QToolButton* m_BtnOpen;
  class QToolButton* m_BtnEject;
};

#endif // USBDLGITEM_HPP