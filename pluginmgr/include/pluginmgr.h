#ifndef PLUGINMGR_H
#define PLUGINMGR_H

#include <map>
#include <string>
#include <filesystem>
#include <functional>

class PluginObject;

class PluginMgr {
public:
  explicit PluginMgr(class GlbObject*, 
    class QMenu* pluginManiMenu);
  ~PluginMgr();
  bool HasPlaugin(PluginObject* plugin);
  void WritteSettings();
  class YJson* GetSettings(const char8_t* key);
  void InitSettingMenu(QMenu* settingsMenu);
  std::function<void()> SaveSettings;
  std::map<std::u8string, class QObject*> m_MainObjects;
  class GlbObject* const m_GlbObject;
  QMenu* const m_MainMenu;
private:
  PluginObject* LoadPlugin(const std::u8string& path);
  void FreePlugin(PluginObject*& plugin);
  bool LoadPlugEnv(const std::filesystem::path& path);
  void LoadPlugins();
  void InitBroadcast();
  const std::u8string m_SettingFileName;
  class YJson* m_Settings;
  std::map<std::u8string, PluginObject*> m_Plugins;
  std::map<PluginObject*, void*> m_PluginPath;
};

#endif // PLUGINMGR_H
