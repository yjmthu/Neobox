#ifndef PLUGINMGR_H
#define PLUGINMGR_H

#include <map>
#include <string>
#include <filesystem>
#include <functional>
#include <mutex>

class NeoMenu;
class YJson;
class PluginObject;
class MenuBase;
class QSharedMemory;
class NeoSystemTray;
class NeoMsgDlg;

class PluginMgr {
public:
  struct PluginInfo {
    PluginObject* plugin = nullptr;
    void* handle = nullptr;
  };
  mutable std::mutex m_Mutex;
public:
  explicit PluginMgr();
  ~PluginMgr();
public:
  void LoadPlugins();
  void LoadManageAction();
  YJson& GetPluginsInfo();
  YJson& GetEventMap();
  YJson& GetNetProxy();
  bool InstallPlugin(const std::u8string& plugin, const YJson& info);
  bool UnInstallPlugin(const std::u8string& plugin);
  bool UpdatePlugin(const std::u8string& plugin, const YJson* info);
  bool TooglePlugin(const std::u8string& plugin, bool on);
  void UpdatePluginOrder(YJson&& data);
  bool IsPluginEnabled(const std::u8string& plugin) const;
private:
  friend class NeoConfig;
  static YJson* InitSettings();
  void SaveSettings();
  bool LoadPlugin(std::u8string pluginName, PluginInfo& info);
  bool FreePlugin(PluginInfo& info);
  bool LoadPlugEnv(const std::filesystem::path& path);
  void InitBroadcast();
  void UpdateBroadcast(PluginObject* plugin);
public:
  QSharedMemory* const m_SharedMemory;
  NeoSystemTray* const m_Tray;
  NeoMenu* const m_Menu;        // 菜单靠后加载
  NeoMsgDlg* const m_MsgDlg;
  // MenuBase* const m_PluginMainMenu;
private:
  friend class TabHotKey;
  static const std::u8string m_SettingFileName;
  YJson* const m_Settings;
  class NeoTimer* const m_SharedTimer;
  class Shortcut* const m_Shortcut;
public:
  class PluginUpdate* const m_UpdateMgr;
  std::map<std::u8string, PluginInfo> m_Plugins;
  std::map<std::u8string, class QObject*> m_MainObjects;
public:
  static void WriteSharedFlag(QSharedMemory*, int flag);
  static int ReadSharedFlag(QSharedMemory*);

  void ShowMsg(class QString text);
  void ShowMsgbox(const std::wstring& title,
    const std::wstring& text, int type = 0);
  static int Exec();
  static void Quit();
  void Restart();
  static std::filesystem::path GetJunkDir();
  static std::filesystem::path GetPluginDir();
private:
  static QSharedMemory* CreateSharedMemory();
  void DetachSharedMemory();
};

extern PluginMgr* mgr;

#endif // PLUGINMGR_H
