#ifndef PLUGINOBJECT_H
#define PLUGINOBJECT_H

#include <neobox/pluginevent.h>

#include <string>
#include <map>
#include <set>
#include <list>
#include <functional>

class PluginObject {
public:
  typedef std::function<void(PluginEvent, void*)> FollowerFunction;
  struct FunctionInfo {
    std::u8string friendlyName;
    std::u8string description;
    FollowerFunction function;
    PluginEvent type;
  };
  typedef std::map<std::u8string, FunctionInfo> FunctionMap;
protected:
  virtual void InitFunctionMap() = 0;
  void AddMainObject(class QObject* object);
  void RemoveMainObject();
protected:
  class YJson& m_Settings;
public:
  FunctionMap m_PluginMethod;
public:
  std::list<std::pair<std::u8string, FollowerFunction>> m_Following;
  std::set<const FollowerFunction*> m_Followers;
  std::u8string const m_PluginName;
  class QAction* const m_MainAction;
  class MenuBase* const m_MainMenu;
public:
  explicit PluginObject(class YJson& settings,
    std::u8string name, const std::u8string& friendlyName);
  virtual ~PluginObject();
public:
  void SendBroadcast(PluginEvent event, void* data);
  virtual class QAction* InitMenuAction();
  static std::u8string QString2Utf8(const class QString& str);
  static QString Utf82QString(const std::u8string& str);
  static QObject* GetMainObject(const std::u8string& pluginName);
};

#endif //PLUGINOBJECT_H
