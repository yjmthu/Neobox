#include <pluginmgr.h>
#include <pluginobject.h>
#include <yjson.h>
#include <systemapi.h>
#include <shortcut.h>
#include <httplib.h>
#include <menubase.hpp>
#include <config.h>

#include <QAction>
#include <QMessageBox>
#include <QActionGroup>
#include <QApplication>
#include <QProcess>
#include <QSharedMemory>

#include <cstdlib>
#include <filesystem>

#include "../widgets/plugincenter.hpp"
#include <neomsgdlg.hpp>
#include <neosystemtray.hpp>
#include <neomenu.hpp>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;
PluginMgr *mgr;

const std::u8string PluginMgr::m_SettingFileName(u8"PluginSettings.json");

// #define WRITE_LOG(x) writelog((x))
#define WRITE_LOG(x) 

void writelog(std::string data)
{
  std::ofstream file("neobox.log", std::ios::app);
  file.write(data.data(), data.size());
  file.close();
}

YJson* PluginMgr::InitSettings()
{
  if (!fs::exists("plugins")) {
    fs::create_directory("plugins");
  }
  YJson* setting = nullptr;
  if (fs::exists(m_SettingFileName)) {
    setting = new YJson(m_SettingFileName, YJson::UTF8);
  } else {
    setting = new YJson{ YJson::O{
      { u8"Plugins", YJson::O {
      }},
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
      { u8"PluginsConfig", YJson::O {
      }},
      { u8"NetProxy", YJson::O {
        {u8"Type", HttpLib::m_Proxy.type},
        {u8"Proxy", HttpLib::m_Proxy.proxy},
        {u8"Username", YJson::String},
        {u8"Password", YJson::String}
      }},
    }};
  }
  auto& proxy = setting->operator[](u8"NetProxy");
  auto& version = setting->operator[](u8"Version");

  if (!version.isArray()) {
    version = YJson::A {
      NEOBOX_VERSION_MAJOR,
      NEOBOX_VERSION_MINOR,
      NEOBOX_VERSION_PATCH
    };
    HttpLib::m_Proxy.GetSystemProxy();

    proxy = YJson::O {
      {u8"Type", HttpLib::m_Proxy.type},
      {u8"Proxy", HttpLib::m_Proxy.proxy},
      {u8"Username", HttpLib::m_Proxy.username},
      {u8"Password", HttpLib::m_Proxy.password}
    };
  } else if (proxy.isObject() && proxy[u8"Proxy"].isNull()) {
    proxy[u8"Proxy"] = HttpLib::m_Proxy.proxy;
  }


  HttpLib::m_Proxy.proxy = proxy[u8"Proxy"].getValueString();
  HttpLib::m_Proxy.username = proxy[u8"Username"].getValueString();
  HttpLib::m_Proxy.password = proxy[u8"Password"].getValueString();
  HttpLib::m_Proxy.type = proxy[u8"Type"].getValueInt();

  return setting;
}


PluginMgr::PluginMgr()
  : m_SharedMemory(CreateSharedMemory())
  , m_Tray(new NeoSystemTray)
  , m_Menu(new NeoMenu)
  , m_MsgDlg(new NeoMsgDlg(m_Menu))
  , m_Settings(InitSettings())
{
  mgr = this;
  m_Tray->setContextMenu(m_Menu);
  m_Tray->show();
  // QObject::connect(m_Menu->addAction("托盘图标"), &QAction::triggered, m_Tray, &QSystemTrayIcon::show);

  m_Shortcut = new Shortcut(m_Settings->find(u8"EventMap")->second);

  LoadPlugins();
  LoadManageAction();
}

PluginMgr::~PluginMgr()
{
  delete m_Shortcut;
  for (auto& [_, info]: m_Plugins) {
    if (!info.plugin) continue;
    FreePlugin(info);
  }
  delete m_Settings;

  delete m_Menu;
  delete m_Tray;

  DetachSharedMemory();   // 在构造函数抛出异常后析构函数将不再被调用
}

void PluginMgr::SaveSettings()
{
  static std::atomic_bool working = false;
  while (working) ;
  working = true;
  m_Settings->toFile(m_SettingFileName, false, YJson::UTF8);
  working = false;
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
#ifndef _DEBUG
  fs::path path = u8"plugins";
  path /= pluginName;
#else
  fs::path path = __FILE__;
  path = path.parent_path().parent_path().parent_path() / "install/lib";
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
  auto hdll = dlopen(cPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
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
  delete info.plugin;
  info.plugin = nullptr;
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
#else
  constexpr char seperator = ':';
  auto const pathEnv = std::getenv("PATH");
#endif
  std::string strEnvPaths(pathEnv ? pathEnv : "");  
  // auto const dwSize = GetEnvironmentVariableW(varName, strEnvPaths.data(), strEnvPaths.size());
  strEnvPaths.pop_back();
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
  auto const bRet = setenv("PATH", strEnvPaths.data(), 1) == 0;
  // ShowMsg(QStringLiteral("代码%1").arg(bRet));
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

void PluginMgr::ShowMsgbox(const std::u8string& title,
                 const std::u8string& text,
                 int type) {
  QMetaObject::invokeMethod(m_Menu, [=](){
    QMessageBox::information(m_Menu,
                             QString::fromUtf8(title.data(), title.size()),
                             QString::fromUtf8(text.data(), text.size()));
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

static void CompareJson(YJson& jsDefault, YJson& jsUser)
{
  if (!jsDefault.isSameType(&jsUser))
    return;
  if (!jsUser.isObject()) {
    YJson::swap(jsDefault, jsUser);
    return;
  }
  for (auto& [key, val]: jsUser.getObject()) {
    auto iter = jsDefault.find(key);
    if (iter != jsDefault.endO()) {
      CompareJson(iter->second, val);
    } else {
      YJson::swap(jsDefault[key], val);
    }
  }
}

void PluginMgr::WriteSharedFlag(QSharedMemory* sharedMemory, int flag) {
  //m_SharedMemory->setKey(QStringLiteral("__Neobox__"));
  sharedMemory->lock();
  *reinterpret_cast<int *>(sharedMemory->data()) = flag;
  sharedMemory->unlock();
}

int PluginMgr::ReadSharedFlag(class QSharedMemory* sharedMemory) {
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
      WriteSharedFlag(sharedMemory, 2);
      sharedMemory->detach();
      delete sharedMemory;
      throw std::runtime_error("Already have an instance.");
    default:
      break;
    }
  } else if (!sharedMemory->create(sizeof(int))) {
    throw std::runtime_error("Already have an instance.");
  }
  WriteSharedFlag(sharedMemory, 0);
  return sharedMemory;
}

void PluginMgr::DetachSharedMemory()
{
  // m_SharedMemory->setKey(QStringLiteral("__Neobox__"));
  m_SharedMemory->detach();
}
