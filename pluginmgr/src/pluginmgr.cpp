#include <pluginmgr.h>
#include <pluginobject.h>
#include <yjson.h>
#include <neoapp.h>
#include <systemapi.h>

#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QActionGroup>

#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

// #define WRITE_LOG(x) writelog((x))
#define WRITE_LOG(x) 

void writelog(std::string data)
{
  std::ofstream file("neobox.log", std::ios::app);
  file.write(data.data(), data.size());
  file.close();
}

PluginMgr* mgr;

PluginMgr::PluginMgr(GlbObject* glb, QMenu* pluginMainMenu):
  m_GlbObject(glb),
  m_SettingFileName(u8"PluginSettings.json"),
  SaveSettings([this](){m_Settings->toFile(m_SettingFileName);}),
  m_MainMenu(pluginMainMenu)
{
  mgr = this;
  if (!fs::exists("plugins")) {
    fs::create_directory("plugins");
  }
  if (fs::exists(m_SettingFileName)) {
    m_Settings = new YJson(m_SettingFileName, YJson::UTF8);
  } else {
    m_Settings = new YJson{ YJson::O{
      { u8"Plugins", YJson::O {
        {u8"neospeedboxplg", YJson::O {
          {u8"Enabled", true},
          {u8"FriendlyName", u8"网速悬浮"},
          {u8"Description", u8"实时显示网速和内存占用"},
          {u8"Author", u8"yjmthu"},
          {u8"Version", YJson::A { 0, 0, 1 }},
        }},
        {u8"neotranslateplg", YJson::O{
          {u8"Enabled", true},
          {u8"FriendlyName", u8"极简翻译"},
          {u8"Description", u8"有道和百度翻译"},
          {u8"Author", u8"yjmthu"},
          {u8"Version", YJson::A { 0, 0, 1 }},
        }},
        {u8"neoocrplg", YJson::O {
          {u8"Enabled", true},
          {u8"FriendlyName", u8"文字识别"},
          {u8"Description", u8"识别截图中的文字"},
          {u8"Author", u8"yjmthu"},
          {u8"Version", YJson::A { 0, 0, 1 }},
        }},
        {u8"neowallpaperplg", YJson::O {
          {u8"Enabled", true},
          {u8"FriendlyName", u8"壁纸引擎"},
          {u8"Description", u8"多种壁纸来源，随心更换壁纸"},
          {u8"Author", u8"yjmthu"},
          {u8"Version", YJson::A { 0, 0, 1 }},
        }},
        {u8"neosystemplg", YJson::O {
          {u8"Enabled", true},
          {u8"FriendlyName", u8"系统控制"},
          {u8"Description", u8"使系统更加方便的一些小功能"},
          {u8"Author", u8"yjmthu"},
          {u8"Version", YJson::A { 0, 0, 1 }},
        }},
        {u8"neohotkeyplg", YJson::O {
          {u8"Enabled", true},
          {u8"FriendlyName", u8"热键管理"},
          {u8"Description", u8"插件热键管理器"},
          {u8"Author", u8"yjmthu"},
          {u8"Version", YJson::A { 0, 0, 1 }},
        }},
      }},
      { u8"EventMap", YJson::A {
        // YJson::O {
        //   {u8"from", u8"" },
        //   {u8"event", u8"" },
        //   {u8"to", u8"" },
        //   {u8"function", u8""},
        // }
      }},
      { u8"PluginsConfig", YJson::O {
      }},
    }};
  }
}

PluginMgr::~PluginMgr()
{
  for (auto& [_, info]: m_Plugins) {
    if (!info.plugin) continue;
    delete info.plugin;
    info.plugin = nullptr;
    FreeLibrary(reinterpret_cast<HINSTANCE>(info.handle));
  }
  delete m_Settings;
}

void PluginMgr::LoadEventMap(QAction *action)
{
  //
}

void PluginMgr::LoadPlugins(QMenu* settingsMenu)
{
  for (auto& [i, j]: m_Settings->find(u8"Plugins")->second.getObject()) {
    const auto name = i;
    const auto& friendlyName = j[u8"FriendlyName"].getValueString();
    auto const action = settingsMenu->addAction(PluginObject::Utf82QString(friendlyName));
    action->setCheckable(true);
    if (j[u8"Enabled"].isTrue()) {
      auto& pluginSttings = m_Settings->find(u8"PluginsConfig")->second[name];
      auto& info = m_Plugins[name];
      if ((info.plugin = LoadPlugin(name))) {
        info.plugin->InitMenuAction();
        action->setChecked(true);
      } else {
        action->setChecked(false);
      }
    } else {
      action->setChecked(false);
    }
    QObject::connect(action, &QAction::triggered, settingsMenu, [this, action, name](bool on){
      auto& info = m_Plugins[name];
      m_Settings->find(u8"Plugins")->second[name][u8"Enabled"] = on;
      SaveSettings();
      if (on) {
        if ((info.plugin = LoadPlugin(name))) {
          info.plugin->InitMenuAction();
          UpdateBroadcast(info.plugin);
        } else {
          action->setChecked(false);
          m_GlbObject->glbShowMsg("设置失败！");
          return;
        }
      } else {
        FreePlugin(info);
      }
      m_GlbObject->glbShowMsg("设置成功！");
    });
  }
  SaveSettings();
  InitBroadcast();
}

PluginObject* PluginMgr::LoadPlugin(const std::u8string& pluginName)
{
  PluginObject* (*newPlugin)(YJson&, PluginMgr*)= nullptr;

#ifdef _DEBUG
  fs::path path = __FILEW__;
  path = path.parent_path().parent_path().parent_path() / "build/plugins";
#else
  fs::path path = u8"plugins";
  path /= pluginName;
  if (!LoadPlugEnv(path)) {
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件文件夹加载失败！"));
    return nullptr;
  }
#endif
  path /= pluginName + u8".dll";
  if (!fs::exists(path)) {
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件文件加载失败！"));
    return nullptr;
  }
  path.make_preferred();
  std::wstring wPath = path.wstring();
  wPath.push_back(L'\0');
  HINSTANCE hdll = LoadLibraryW(wPath.data());
  if (!hdll) {
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件动态库加载失败！"));
    return nullptr;
  }
  newPlugin = reinterpret_cast<decltype(newPlugin)>(GetProcAddress(hdll, "newPlugin"));
  if (!newPlugin) {
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件函数加载失败！"));
    FreeLibrary(hdll);
    return nullptr;
  }
  try {
    return newPlugin(m_Settings->find(u8"PluginsConfig")->second[pluginName], this);      // nice
  } catch (...) {
    glb->glbShowMsg(PluginObject::Utf82QString(path.u8string() + u8"插件初始化失败！"));
  }
  return nullptr;
}

void PluginMgr::FreePlugin(PluginInfo& info)
{
  delete info.plugin;
  info.plugin = nullptr;
  FreeLibrary(reinterpret_cast<HINSTANCE>(info.handle));
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
    WRITE_LOG("dir not exsist\n");
    return false;
  }
  constexpr auto const varName = L"PATH";
  std::wstring strEnvPaths(GetEnvironmentVariableW(varName, nullptr, 0), L'\0');  
  DWORD const dwSize = GetEnvironmentVariableW(varName, strEnvPaths.data(), strEnvPaths.size());
  strEnvPaths.pop_back();
  if (!strEnvPaths.ends_with(L';'))
    strEnvPaths.push_back(L';');
  auto path = fs::absolute(dir);
  path.make_preferred();
  auto const & wpath = path.wstring();
  if (auto pos = strEnvPaths.find(wpath); pos != std::wstring::npos) {
    pos += wpath.size();
    if (wpath.size() == pos || wpath[pos] == L';') {
      return true;
    }
  }
  strEnvPaths.append(wpath);
  strEnvPaths.push_back(L'\0');
  BOOL const bRet = SetEnvironmentVariableW(varName, strEnvPaths.data());  
  return bRet;  
}
