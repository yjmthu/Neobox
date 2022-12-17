#ifndef PLUGINMGR_H
#define PLUGINMGR_H

#include <map>
#include <string>
#include <filesystem>
#include <functional>

class PluginObject;

class PluginMgr {
public:
  struct PluginInfo {
    PluginObject* plugin = nullptr;
    void* handle = nullptr;
  };
public:
  explicit PluginMgr(class GlbObject*, 
    class QMenu* pluginManiMenu);
  ~PluginMgr();
  bool HasPlaugin(PluginObject* plugin);
  void WritteSettings();
  class YJson* GetSettings(const char8_t* key);
  void LoadPlugins(QMenu* settingsMenu);
  void LoadManageAction(class QAction *action);
  const class YJson& GetPluginsInfo() const;
private:
  PluginObject* LoadPlugin(const std::u8string& path);
  void FreePlugin(PluginInfo& info);
  bool LoadPlugEnv(const std::filesystem::path& path);
  void InitBroadcast();
  void UpdateBroadcast(PluginObject* plugin);
public:
  std::map<std::u8string, PluginInfo> m_Plugins;
  std::function<void()> SaveSettings;
  std::map<std::u8string, class QObject*> m_MainObjects;
  class GlbObject* const m_GlbObject;
  QMenu* const m_MainMenu;
private:
  const std::u8string m_SettingFileName;
  class YJson* m_Settings;
};

extern PluginMgr* mgr;

#endif // PLUGINMGR_H
