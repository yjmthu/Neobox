#ifndef USBDLGITEM_H
#define USBDLGITEM_H

#include <QWidget>
#include <map>
#include <windows.h>

#include <usbdlg.h>

class UsbDlgItem: public QWidget
{
protected:
  bool eventFilter(QObject*, QEvent*) override;
public:
  explicit UsbDlgItem(QWidget* parent, char id, UsbDlg::ItemMap& map);
  ~UsbDlgItem();
public:
  void PopUsbDrive();
private:
  void SetupUi();
  bool IsDiskExist() const;
  void SetUsbInfoText();
  bool UpdateUsbSize();
  bool UpdateUsbName();
  QString GetStyleSheet() const;
  void SetStyleSheet();
  static std::wstring FormatSize(uint64_t size);
  bool EjectUsbDisk();
  DWORD GetDrivesDevInstByDiskNumber(const DWORD diskNumber);

private:
  UsbDlg::ItemMap& m_Items;
  const wchar_t m_DriveId;
  const wchar_t* const m_DrivePath;
  std::wstring m_DriveName;
  uint64_t m_SizeTotal;
  uint64_t m_SizeFree;
  QWidget* const m_UsbSizeLogo;
  class QFrame* const m_UsbSizeLogoColorMask;
  class QLabel* const m_UsbInfoText;
};

#endif // USBDLGITEM_H