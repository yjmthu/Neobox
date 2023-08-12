#include <pluginmgr.h>
#include <pluginobject.h>
#include <yjson.h>
#include <systemapi.h>
#include <shortcut.h>
#include <httplib.h>
#include <menubase.hpp>
#include <config.h>
#include <neotimer.h>

#include <QAction>
#include <QMessageBox>
#include <QActionGroup>
#include <QApplication>
#include <QProcess>
#include <QSharedMemory>

#include <cstdlib>
#include <filesystem>

#include <update.hpp>
#include <neomsgdlg.hpp>
#include <neosystemtray.hpp>
#include <neomenu.hpp>
#include "../widgets/plugincenter.hpp"

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;
using namespace std::literals;
PluginMgr *mgr;

const std::u8string PluginMgr::m_SettingFileName(u8"PluginSettings.json");

YJson* PluginMgr::InitSettings()
{
  YJson* setting = nullptr;
  try {
    setting = new YJson(m_SettingFileName, YJson::UTF8);
  } catch (std::runtime_error err) {
    delete setting;
    setting = new YJson(YJson::O {
      { u8"Plugins", YJson::Object },
      { u8"EventMap", YJson::A {
        // YJson::O {
        //   {u8"KeySequence", u8"Shift+A"},
        //   {u8"Enabled", false},
        //   {u8"Plugin", YJson::O {
        //     {u8"PluginName", u8"neospeedboxplg"},
        //     {u8"Function", u8"show"},
        //   }},
        // },
        // YJson::O {
        //   {u8"KeySequence", u8"Shift+S"},
        //   {u8"Enabled", false},
        //   {u8"Command", YJson::O {
        //     {u8"Executable", u8"shutdown.exe"},
        //     {u8"Directory", u8"."},
        //     {u8"Arguments", YJson:A {u8"-s", u8"-t", u8"10"}}
        //   }},
        // }
      }},
      { u8"PluginsConfig", YJson::Object },
      { u8"NetProxy", nullptr },
    });
  }
  auto& proxy = setting->operator[](u8"NetProxy");
  auto& version = setting->operator[](u8"Version");

  if (!version.isArray()) {
    version = YJson::A {
      NEOBOX_VERSION_MAJOR,
      NEOBOX_VERSION_MINOR,
      NEOBOX_VERSION_PATCH
    };
  }

  HttpLib::m_Proxy.emplace(proxy);
  return setting;
}

PluginMgr::PluginMgr()
  : m_SharedMemory(CreateSharedMemory())
  , m_Tray(new NeoSystemTray)
  , m_Menu(new NeoMenu)
  , m_MsgDlg(new NeoMsgDlg(m_Menu))
  , m_Settings((mgr = this, InitSettings()))
  , m_SharedTimer(new NeoTimer)
  , m_UpdateMgr(new PluginUpdate((*m_Settings)[u8"Upgrade"]))
  , m_Shortcut { new Shortcut(m_Settings->find(u8"EventMap")->second) }
{
  m_Tray->setContextMenu(m_Menu);
  m_Tray->show();
  // QObject::connect(m_Menu->addAction("托盘图标"), &QAction::triggered, m_Tray, &QSystemTrayIcon::show);
  m_SharedTimer->StartTimer(1s, [this](){
    if (ReadSharedFlag(m_SharedMemory) == -1) {
      QMetaObject::invokeMethod(m_Menu, QApplication::quit);
      WriteSharedFlag(m_SharedMemory, 1);
    }
  });

  LoadPlugins();
  LoadManageAction();
}

PluginMgr::~PluginMgr()
{
  delete m_UpdateMgr;

  for (auto& [_, info]: m_Plugins) {
    if (!info.plugin) continue;
    FreePlugin(info);
  }
  // 先释放插件，再释放热键
  delete m_Shortcut;

  delete m_Settings;

  delete m_Menu;
  delete m_Tray;
  delete m_SharedTimer;

  DetachSharedMemory();   // 在构造函数抛出异常后析构函数将不再被调用
}

void PluginMgr::SaveSettings()
{
  // std::lock_guard<std::mutex> locker(m_Mutex);
  m_Settings->toFile(m_SettingFileName, false, YJson::UTF8);
}

void PluginMgr::LoadManageAction()
{
  QObject::connect(m_Menu->m_ControlPanel, &QAction::triggered, m_Menu, [this](){
    auto instance = PluginCenter::m_Instance;
    if (instance) {
      instance->activateWindow();
      return;
    }
    auto const center = new PluginCenter;
    center->show();
  });
}

YJson& PluginMgr::GetPluginsInfo()
{
  return m_Settings->find(u8"Plugins")->second;
}

YJson& PluginMgr::GetEventMap() {
  return m_Settings->find(u8"EventMap")->second;
}

YJson& PluginMgr::GetNetProxy()
{
  return m_Settings->find(u8"NetProxy")->second;
}

void PluginMgr::LoadPlugins()
{
  for (auto& [i, j]: m_Settings->find(u8"Plugins")->second.getObject()) {
    const auto name = i;
    const char* data = (const char*) name.c_str();
    if (!j[u8"Enabled"].isTrue()) continue;
    auto& pluginSttings = m_Settings->find(u8"PluginsConfig")->second[name];
    if (!LoadPlugin(name, m_Plugins[name])) {
      m_Plugins.erase(name);
      j[u8"Enabled"] = false;
    }
  }
  SaveSettings();
  InitBroadcast();
}

bool PluginMgr::TooglePlugin(const std::u8string& pluginName, bool on)
{
  auto& info = m_Plugins[pluginName];
  auto& enabled = m_Settings->find(u8"Plugins")->second[pluginName][u8"Enabled"];
  enabled = on;
  SaveSettings();      // 尽量早保存，以免闪退

  if (on) {
    if (LoadPlugin(pluginName, info)) {
      UpdateBroadcast(info.plugin);
    } else {
      ShowMsg("设置失败！");
      m_Plugins.erase(pluginName);
      enabled = false;
      return false;
    }
  } else {
    FreePlugin(info);
    m_Plugins.erase(pluginName);
  }
  ShowMsg("设置成功！");
  return true;
}

bool PluginMgr::LoadPlugin(std::u8string pluginName, PluginMgr::PluginInfo& pluginInfo)
{
  pluginInfo.plugin = nullptr;
  pluginInfo.handle = nullptr;

  PluginObject* (*newPlugin)(YJson&, PluginMgr*)= nullptr;
#ifdef _RELEASE
  fs::path path = GetPluginDir() / pluginName;
#else
  fs::path path = __FILE__;
  path = path.parent_path().parent_path().parent_path() / "build/Debug/plugins" / pluginName;
  path.make_preferred();
#endif
  if (!LoadPlugEnv(path)) {
    ShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件文件夹加载失败！"));
    return false;
  }
#ifdef _WIN32
  path /= pluginName + u8".dll";
#else
  path /= u8"lib" + pluginName + u8".so";
#endif
  if (!fs::exists(path)) {
    ShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件文件加载失败！"));
    return false;
  }
  path.make_preferred();

#ifdef _WIN32
  auto wPath = path.wstring();
  wPath.push_back(L'\0');
  HINSTANCE hdll = LoadLibraryW(wPath.data());
#else
  auto cPath = path.string();
  auto hdll = dlopen(cPath.c_str(), RTLD_LAZY);
#endif

  if (!hdll) {
    ShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件动态库加载失败！"));
    return false;
  }
  pluginInfo.handle = hdll;
  newPlugin = reinterpret_cast<decltype(newPlugin)>
#ifdef _WIN32
  (GetProcAddress(hdll, "newPlugin"));
#else
  (dlsym(hdll, "newPlugin"));
#endif
  if (!newPlugin) {
    ShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件函数加载失败！"));
    FreePlugin(pluginInfo);
    return false;
  }
  try {
    pluginInfo.plugin = newPlugin(m_Settings->find(u8"PluginsConfig")->second[pluginName], this);      // nice
    auto const mainMenuAction = pluginInfo.plugin->InitMenuAction();
    if (mainMenuAction) {
      mainMenuAction->setProperty("pluginName", QString::fromUtf8(pluginName.data(), pluginName.size()));
      m_Menu->addAction(mainMenuAction);
    }
    m_Shortcut->RegisterPlugin(pluginInfo.plugin->m_PluginName);
    return true;
  } catch (...) {
    // pluginInfo.plugin = nullptr;
    FreePlugin(pluginInfo);
    pluginInfo.handle = nullptr;
    ShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件初始化失败！"));
  }
  return false;
}

bool PluginMgr::FreePlugin(PluginInfo& info)
{
  if (info.plugin) {
    m_Shortcut->UnregisterPlugin(info.plugin->m_PluginName);
    delete info.plugin;
    info.plugin = nullptr;
  }
#ifdef _WIN32
  return FreeLibrary(reinterpret_cast<HINSTANCE>(info.handle));
#else
  return dlclose(info.handle) == 0;
#endif
}

void PluginMgr::InitBroadcast()
{
  for (auto& [name, info]: m_Plugins) {
    if (!info.plugin) continue;
    for (const auto& idol: info.plugin->m_Following) {
      auto iter = m_Plugins.find(idol.first);
      if (iter == m_Plugins.end() || !iter->second.plugin) continue;
      iter->second.plugin->m_Followers.insert(&idol.second);
    }
  }
}

void PluginMgr::UpdateBroadcast(PluginObject* plugin)
{
  for (auto& [name, info]: m_Plugins) {
    if (!info.plugin || info.plugin == plugin) continue;
    auto const & lst = info.plugin->m_Following;
    auto iter = std::find_if(lst.begin(), lst.end(),
      [plugin](decltype(lst.front())& item){
        return item.first == plugin->m_PluginName;
      });
    if (iter == lst.end()) continue;
    plugin->m_Followers.insert(&iter->second);
  }
  for (auto& idol: plugin->m_Following) {
    auto iter = m_Plugins.find(idol.first);
    if (iter == m_Plugins.end() || !iter->second.plugin) continue;
    iter->second.plugin->m_Followers.insert(&idol.second);
  }
}

bool PluginMgr::LoadPlugEnv(const fs::path& dir)
{
  if (!fs::exists(dir)) {
    ShowMsg("加载插件时找不到文件夹");
    return false;
  }
#ifdef _WIN32
  constexpr char seperator = ';';
  auto const dwSize = GetEnvironmentVariableA("PATH", nullptr, 0); // GetEnvironmentVariableA(varName, strEnvPaths.data(), strEnvPaths.size());
  std::string strEnvPaths(dwSize, 0);
  auto const pathEnv = GetEnvironmentVariableA("PATH", strEnvPaths.data(), dwSize);
  if (strEnvPaths.ends_with('\0'))
    strEnvPaths.pop_back();
#else
  constexpr char seperator = ':';
  auto const pathEnv = std::getenv("PATH");
  std::string strEnvPaths(pathEnv ? pathEnv : "");
#endif
  if (!strEnvPaths.empty() && !strEnvPaths.ends_with(seperator))
    strEnvPaths.push_back(seperator);
  auto path = fs::absolute(dir);
  path.make_preferred();
  auto cpath = path.string();
  for (auto pos = strEnvPaths.find(cpath); pos != std::string::npos; pos = strEnvPaths.find(cpath, pos)) {
    pos += cpath.size();
    if (pos == strEnvPaths.size() || strEnvPaths[pos] == seperator) {
      return true;
    }
  }
  cpath.push_back('\0');
  strEnvPaths.append(cpath);
#ifdef _WIN32
  auto const bRet = SetEnvironmentVariableA("PATH", strEnvPaths.data());
#else
  auto const bRet = setenv("PATH", strEnvPaths.data(), 1) == 0;
#endif
  return bRet;
}


bool PluginMgr::InstallPlugin(const std::u8string& plugin, const YJson& info)
{
  auto iter = m_Plugins.find(plugin);
  if (iter != m_Plugins.end() && iter->second.plugin) {
    return false;
  }
  auto& pluginsInfo = m_Settings->find(u8"Plugins")->second;
  auto infoIter = pluginsInfo.find(plugin);

  if (infoIter != pluginsInfo.endO()) {
    return false;
  }

  auto& pluginInfo = pluginsInfo[plugin] = info;
  pluginInfo[u8"Enabled"] = false;
  SaveSettings();
  return true;
}

bool PluginMgr::UnInstallPlugin(const std::u8string& plugin)
{
  auto iter = m_Plugins.find(plugin);
  if (iter != m_Plugins.end() && iter->second.plugin) {
    FreePlugin(iter->second);
    m_Plugins.erase(iter);
  }
  auto& pluginsInfo = m_Settings->find(u8"Plugins")->second;
  auto infoIter = pluginsInfo.find(plugin);

  if (infoIter == pluginsInfo.endO()) {
    return false;
  }
  pluginsInfo.remove(infoIter);
  SaveSettings();
  return true;
}


bool PluginMgr::UpdatePlugin(const std::u8string& plugin, const YJson* info)
{
  auto& pluginsInfo = m_Settings->find(u8"Plugins")->second;
  auto infoIter = pluginsInfo.find(plugin);

  if (infoIter == pluginsInfo.endO()) {
    return false;
  }
  auto& pluginInfo = infoIter->second;

  if (info) {
    if (const auto iter = m_Plugins.find(plugin); iter != m_Plugins.end() && iter->second.plugin) {
      return false;
    }
    auto& enabled = (pluginInfo = *info)[u8"Enabled"];
    if (enabled.isTrue()) {
      auto& ptrInfo = m_Plugins[plugin];
      if (LoadPlugin(plugin, ptrInfo)) {
        UpdateBroadcast(ptrInfo.plugin);
      } else {
        enabled = false;
        SaveSettings();
        return false;
      }
    }
    SaveSettings();
  } else {
    if (const auto iter = m_Plugins.find(plugin); iter != m_Plugins.end() && iter->second.plugin) {
      FreePlugin(iter->second);
      m_Plugins.erase(iter);
    }
    pluginInfo[u8"Enabled"] = false;
  }
  return true;
}

void PluginMgr::UpdatePluginOrder(YJson&& data)
{
  auto& pluginsInfo = m_Settings->find(u8"Plugins")->second;
  pluginsInfo = std::move(data);
  std::map<std::u8string, QAction*> actions;

  for (auto action: m_Menu->actions()) {
    auto pluginName = action->property("pluginName");
    if (!pluginName.isNull()) {
      actions[PluginObject::QString2Utf8(pluginName.toString())] = action;
    }
  }

  for (auto& [name, action]: actions) {
    m_Menu->removeAction(action);
  }

  for (auto& [name, info]: pluginsInfo.getObject()) {
    m_Menu->addAction(actions[name]);
  }

  ShowMsg("调整成功！");
  SaveSettings();
}

bool PluginMgr::IsPluginEnabled(const std::u8string& plugin) const
{
  return m_Plugins.find(plugin) != m_Plugins.end();
}

void PluginMgr::ShowMsgbox(const std::wstring& title,
                 const std::wstring& text,
                 int type) {
  QMetaObject::invokeMethod(m_Menu, [=, this](){
    QMessageBox::information(m_Menu, QString::fromStdWString(title),
      QString::fromStdWString(text));
  });
}

void PluginMgr::ShowMsg(class QString text)
{
  m_MsgDlg->ShowMessage(text);
}

int PluginMgr::Exec()
{
  return QApplication::exec();
}

void PluginMgr::Quit()
{
  QApplication::quit();
}

void PluginMgr::Restart()
{
  WriteSharedFlag(m_SharedMemory, 1);
  QProcess::startDetached(
    QApplication::applicationFilePath(), QStringList {}
  );
  QApplication::quit();
}

fs::path PluginMgr::GetJunkDir() {
  fs::path result = "junk";
  // 必须成功，不成功则直接抛异常
  fs::create_directory(result);
  return fs::absolute(result);
}

fs::path PluginMgr::GetPluginDir() {
  fs::path result = "plugins";
  // 必须成功，不成功则直接抛异常
  fs::create_directory(result);
  return fs::absolute(result);
}

void PluginMgr::WriteSharedFlag(QSharedMemory* sharedMemory, int flag) {
  //m_SharedMemory->setKey(QStringLiteral("__Neobox__"));
  sharedMemory->lock();
  *reinterpret_cast<int *>(sharedMemory->data()) = flag;
  sharedMemory->unlock();
}

int PluginMgr::ReadSharedFlag(QSharedMemory* sharedMemory) {
  //m_SharedMemory->setKey(QStringLiteral("__Neobox__"));
  sharedMemory->lock();
  const auto state = *reinterpret_cast<const int*>(sharedMemory->constData());
  sharedMemory->unlock();
  return state;
}

QSharedMemory* PluginMgr::CreateSharedMemory() {
  QSharedMemory* sharedMemory = new QSharedMemory(QStringLiteral("__Neobox__"));
  if(sharedMemory->attach()) {
    auto const code = ReadSharedFlag(sharedMemory);
    switch (code) {
    case 0:   //  already have an instance;
      // WriteSharedFlag(sharedMemory, 2);
      sharedMemory->detach();
      delete sharedMemory;
      throw std::runtime_error("There is already an instance running!");
    default:
      break;
    }
  } else if (!sharedMemory->create(sizeof(int))) {
    throw std::runtime_error("Unable to create Shared memory!");
  }
  WriteSharedFlag(sharedMemory, 0);
  return sharedMemory;
}

void PluginMgr::DetachSharedMemory()
{
  // m_SharedMemory->setKey(QStringLiteral("__Neobox__"));
  m_SharedMemory->detach();
}
