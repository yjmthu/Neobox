#ifndef NEOUSBPLG_H
#define NEOUSBPLG_H

#include <pluginobject.h>
#include <QAbstractNativeEventFilter>
#include <map>

#include <usbconfig.h>

#ifdef _WIN32
class NeoUsbPlg: public QAbstractNativeEventFilter, public PluginObject
#else
class NeoUsbPlg: public PluginObject
#endif
{
protected:
#ifdef _WIN32
  bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;
#endif
  class QAction* InitMenuAction() override;
public:
  explicit NeoUsbPlg(YJson& settings);
  virtual ~NeoUsbPlg();
private:
  YJson& InitSettings(YJson& settings);
  void InitFunctionMap() override;
private:
  UsbConfig m_Settings;
  class QAction* m_MainMenuAction;
  class UsbDlg* m_UsbDlg;
  // std::map<char, class UsbDlg*> m_Items;
};

#endif // NEOUSBPLG_H
