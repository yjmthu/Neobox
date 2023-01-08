#ifndef NEOHOTKEYPLG_H
#define NEOHOTKEYPLG_H

#include <pluginobject.h>
#include <QAbstractNativeEventFilter>

class NeoHotKeyPlg: public QAbstractNativeEventFilter, public PluginObject
{
protected:
  bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;
  class QAction* InitMenuAction() override;
  void InitFunctionMap() override;
public:
  explicit NeoHotKeyPlg(YJson& settings);
  virtual ~NeoHotKeyPlg();
private:
  static YJson& InitSettings(YJson& settings);
  void InitMenu();
private:
  class Shortcut* m_Shortcut;
};

#endif // NEOHOTKEYPLG_H