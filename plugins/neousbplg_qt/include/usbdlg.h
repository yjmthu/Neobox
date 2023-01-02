#ifndef USBDLG_H
#define USBDLG_H

#include <QDialog>

class UsbDlg: public QDialog
{
public:
  typedef std::map<char, class UsbDlgItem*> ItemMap;
protected:
  bool nativeEvent(const QByteArray& eventType,
                   void* message,
                   qintptr* result) override;
  void showEvent(QShowEvent*) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
public:
  explicit UsbDlg(class YJson& settings);
  ~UsbDlg();
public:
  static std::string GetDrives(const void* lpdb);
  void DoDeviceArrival(const void* lpdb);
  void DoDeviceRemoveComplete(const void* lpdb);
private:
  void SetupUi();
  void SetupAnimation();
  void SetHideFullScreen();

  void GetUsbInfo();
private:
  QPoint m_ConstPos;
  YJson& m_Settings;
  YJson& m_Position;
  QWidget* m_CenterWidget;
  void* m_AppBarData;
  class QPropertyAnimation* m_Animation;
  class QVBoxLayout* m_MainLayout;
  static ItemMap m_Items;
};

#endif // USBDLG_H