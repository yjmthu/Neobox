#ifndef PLUGINOBJECT_H
#define PLUGINOBJECT_H

#include <pluginevent.h>

#include <string>
#include <map>
#include <vector>
#include <functional>

class PluginObject {
  struct FunctionInfo {
    std::u8string friendlyName;
    std::u8string description;
  };
  static std::map<std::u8string, class QObject*> m_MainObjects;
public:
  struct FunctionInfoVoid: public FunctionInfo {
    std::function<void(void)> function;
  };
  struct FunctionInfoBool: public FunctionInfo {
    std::function<void(bool)> function;
    std::function<bool(void)> status;
  };
  typedef std::function<void(PluginEvent, void*)> FollowerFunction;
  typedef std::map<std::u8string, FunctionInfoVoid> FunctionMapVoid;
  typedef std::map<std::u8string, FunctionInfoBool> FunctionMapBool;
  struct PlugInfo {
    const std::u8string m_FriendlyName;
    const std::u8string m_PluginName;
    const FunctionMapVoid& m_FunctionMapVoid;
    const FunctionMapBool& m_FunctionMapBool;
  };
  const PlugInfo& GetPlugInfo() const
    { return m_PlugInfo; }
protected:
  virtual void InitFunctionMap() {}
  void AddMainObject(QObject* object);
  void RemoveMainObject();
  FunctionMapVoid m_FunctionMapVoid;
  FunctionMapBool m_FunctionMapBool;
  class YJson& m_Settings;
  const PlugInfo m_PlugInfo;
  std::vector<const FollowerFunction*> m_Followers;
  std::vector<std::u8string> m_Following;
public:
  explicit PluginObject(class YJson& settings,
      std::u8string pluginName, std::u8string friendlyName);
  virtual ~PluginObject() = default;
  virtual void InitMenuAction(class QMenu* pluginMenu);
  static std::function<void()> SaveSettings;
  static std::u8string QString2Utf8(const class QString& str);
  static QString Utf82QString(const std::u8string& str);
  static QObject* GetMainObject(const std::u8string& pluginName);
private:
};

#endif //PLUGINOBJECT_H
