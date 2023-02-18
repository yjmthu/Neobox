#ifndef USBDLG_HPP
#define USBDLG_HPP

#include <widgetbase.hpp>
#include <memory>

class UsbDlg: public WidgetBase
{
#ifdef __linux__
  Q_OBJECT
#endif

public:
#ifdef _WIN32
  typedef std::map<char, class UsbDlgItem*> ItemMap;
#else
  typedef std::map<std::string, class UsbDlgItem*> ItemMap;
#endif
protected:
#ifdef _WIN32
  bool nativeEvent(const QByteArray& eventType,
                   void* message,
                   qintptr* result) override;
#endif
  void showEvent(QShowEvent*) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
public:
  explicit UsbDlg(class YJson& settings);
  ~UsbDlg();
#ifdef _WIN32
  static std::string GetDrives(const void* lpdb);
  void DoDeviceArrival(const void* lpdb);
  void DoDeviceRemoveComplete(const void* lpdb);
#else
public slots:
  void DoDeviceArrival(const QString& lpdb);
  void DoDeviceRemoveComplete(const QString& lpdb);
#endif
private:
  void SetupUi();
  void SetupAnimation();
#ifdef _WIN32
  void SetHideFullScreen();
#else
  void MessageLoop();
  // void QuitMessageLoop();
#endif
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
#ifdef __linux__
  int m_NetlinkSocket;
  class QSocketNotifier* m_SocketNotifier;

signals:
  void UsbChange(QString name);
  void UsbAdd(QString name);
  void UsbRemove(QString name);
  
#endif
};

#endif // USBDLG_HPP
