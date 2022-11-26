#ifndef PLUGINMGR_H
#define PLUGINMGR_H

#include <map>
#include <string>

class PluginObject;

class PluginMgr {
public:
  explicit PluginMgr(class QMenu* pluginManiMenu);
  ~PluginMgr();
  bool HasPlaugin(PluginObject* plugin);
  void WritteSettings();
  class YJson* GetSettings(const char8_t* key);
private:
  PluginObject* LoadPlugin(const std::u8string& path);
  void FreePlugin(PluginObject* plugin);
  void LoadPlugins();
  const std::u8string m_SettingFileName;
  QMenu* const m_MainMenu;
  class YJson* m_Settings;
  std::map<std::u8string, PluginObject*> m_Plugins;
  std::map<PluginObject*, void*> m_PluginPath;
};

#endif // PLUGINMGR_H
