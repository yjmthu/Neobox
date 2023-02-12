#ifndef TABHOTKEY_HPP
#define TABHOTKEY_HPP

#include <QWidget>
#include <map>

#include <yjson.h>

namespace Ui {
    class FormHotKey;
} // namespace Ui

struct HotKeyInfoPlugin {
private:
  YJson data;
public:
  std::u8string& pluginName;
  std::u8string& function;
  bool enabled;
public:
  explicit HotKeyInfoPlugin(const YJson& data, bool on);
  YJson GetJson() const { return data; };
};

struct HotKeyInfoCommand {
private:
  YJson data;
public:
  std::u8string& directory;
  YJson::ArrayType& arguments;
  bool enabled;
public:
  explicit HotKeyInfoCommand(const YJson& data, bool on);
  YJson GetJson() const { return data; };
};

class TabHotKey: public QWidget
{
  Q_OBJECT

public:
  typedef std::map<QString, std::unique_ptr<HotKeyInfoCommand>> MapCommand;
  typedef std::map<QString, std::unique_ptr<HotKeyInfoPlugin>> MapPlugin;
protected:
  bool eventFilter(QObject *obj, QEvent *event);
public:
  explicit TabHotKey(class PluginCenter* center);
  virtual ~TabHotKey();
private:
  void InitLayout();
  void InitSignals();
  void InitDataStruct();
  void InitPluginCombox();
  bool IsDataInvalid() const;
private slots:
  void UpdateHotKeyEditor(QString text);
  void UpdatePluginMethord(int index);
  void ChangeEnabled(bool on);
  void ChooseDirectory();
  void EditArgList();
  bool SaveHotKeyData();
  void AddEmptyItem();
  void RemoveItem();
private:
  class YJson& m_Settings;
  class Shortcut& m_Shortcut;
  MapCommand m_Commands;
  MapPlugin m_Plugins;
  Ui::FormHotKey* ui;
  YJson::ArrayType m_ArgList;
  std::vector<std::u8string> m_PluginNames;
};

#endif // TABHOTKEY_HPP
