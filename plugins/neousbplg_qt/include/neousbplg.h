#ifndef NEOUSBPLG_H
#define NEOUSBPLG_H

#include <pluginobject.h>
#include <QAbstractNativeEventFilter>
#include <map>

#include <windows.h>

class NeoUsbPlg: public QAbstractNativeEventFilter, public PluginObject
{
protected:
  bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;
  class QAction* InitMenuAction() override;
public:
  explicit NeoUsbPlg(YJson& settings);
  virtual ~NeoUsbPlg();
private:
  YJson& InitSettings(YJson& settings);
  void InitFunctionMap() override;
private:
  class QAction* m_MainMenuAction;
  class UsbDlg* m_UsbDlg;
  // std::map<char, class UsbDlg*> m_Items;
};

#endif // NEOUSBPLG_H
