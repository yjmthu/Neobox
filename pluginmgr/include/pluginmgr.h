#ifndef PLUGINMGR_H
#define PLUGINMGR_H

#include <map>
#include <string>
#include <filesystem>
#include <functional>

class YJson;
class PluginObject;
class GlbObject;

class PluginMgr {
public:
  struct PluginInfo {
    PluginObject* plugin = nullptr;
    void* handle = nullptr;
  };
public:
  explicit PluginMgr(GlbObject*, class QMenu* pluginManiMenu);
  ~PluginMgr();
  bool HasPlaugin(PluginObject* plugin);
  void WritteSettings();
  YJson* GetSettings(const char8_t* key);
  void LoadPlugins();
  void LoadManageAction(class QAction *action);
  const YJson& GetPluginsInfo() const;
  YJson& GetEventMap();
  YJson& GetNetProxy();
  bool InstallPlugin(const std::u8string& plugin, const YJson& info);
  bool UnInstallPlugin(const std::u8string& plugin);
  bool UpdatePlugin(const std::u8string& plugin, const YJson* info);
  bool TooglePlugin(const std::u8string& plugin, bool on);
  void UpdatePluginOrder(YJson&& data);
  bool IsPluginEnabled(const std::u8string& plugin) const;
private:
  bool LoadPlugin(std::u8string pluginName, PluginInfo& info);
  bool FreePlugin(PluginInfo& info);
  bool LoadPlugEnv(const std::filesystem::path& path);
  void InitBroadcast();
  void UpdateBroadcast(PluginObject* plugin);
public:
  std::map<std::u8string, PluginInfo> m_Plugins;
  std::function<void()> SaveSettings;
  std::map<std::u8string, class QObject*> m_MainObjects;
  GlbObject* const m_GlbObject;
  QMenu* const m_MainMenu;
private:
  const std::u8string m_SettingFileName;
  YJson* m_Settings;
  friend class TabHotKey;
  class Shortcut* m_Shortcut;
};

extern PluginMgr* mgr;

#endif // PLUGINMGR_H
