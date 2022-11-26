#include <pluginmgr.h>
#include <pluginobject.h>
#include <yjson.h>

#ifdef _DEBUG
#include <neospeedboxplg.h>
#endif

#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

PluginMgr::PluginMgr(QMenu* pluginMainMenu):
  m_SettingFileName(u8"PluginSettings.json"),
  m_MainMenu(pluginMainMenu)
{
  if (!fs::exists("plugins")) {
    fs::create_directory("plugins");
  }
  if (fs::exists(m_SettingFileName)) {
    m_Settings = new YJson(m_SettingFileName, YJson::UTF8);
  } else {
    m_Settings = new YJson{ YJson::O{
      { u8"Plugins", YJson::O {
        {u8"neospeedboxplg", true},
        {u8"neotranslateplg", true},
        {u8"neoocrplg", true},
      }},
      { u8"KeyMap", YJson::O {
        {u8"neotranslateplg", YJson::A {
          YJson::A {u8"toggleVisibility", 0, YJson::A {u8"Shift", u8"Z"}},
        }},
        {u8"neoocrplg", YJson::A {
          YJson::A {u8"screenfetch", 0, YJson::A {"Ctrl", u8"Shift", u8"A"}},
        }},
      }},
      { u8"PluginsConfig", YJson::O {
      }},
    }};
  }

  (PluginObject::SaveSettings=[this](){m_Settings->toFile(m_SettingFileName);})();

  LoadPlugins();
}

PluginMgr::~PluginMgr()
{
#ifndef _DEBUG
  for (auto [i, j]: m_PluginPath) {
    if (j) delete i;
    FreeLibrary(reinterpret_cast<HINSTANCE>(j));
  }
#endif
  delete m_Settings;
}

void PluginMgr::LoadPlugins()
{
  for (const auto& [name, on]: m_Settings->find(u8"Plugins")->second.getObject()) {
    if (!on.isTrue()) continue;
    PluginObject* objptr = nullptr;
    auto& pluginSttings = m_Settings->find(u8"PluginsConfig")->second[name];
#ifdef _DEBUG
    if (name == u8"neospeedboxplg") {
      objptr = new NeoSpeedboxPlg((*m_Settings)[name]);
    }
#else
    objptr = LoadPlugin(name);
#endif
    if (!objptr) continue;
    auto& pluginName = objptr->GetPlugInfo().m_FriendlyName;
    QMenu* const pluginMenu = new QMenu(m_MainMenu);
    m_MainMenu->addAction(PluginObject::Utf82QString(pluginName))->setMenu(pluginMenu);
    objptr->InitMenuAction(pluginMenu);
    m_Plugins[name] = objptr;
  }
  PluginObject::SaveSettings();
}

PluginObject* PluginMgr::LoadPlugin(const std::u8string& pluginName)
{
  PluginObject* (*newPlugin)(YJson&) = nullptr;
  fs::path path = u8"plugins";
  path /= pluginName + u8".dll";
  if (!fs::exists(path)) return nullptr;
  // const std::u8string pluginName = path.stem().u8string();
  path.make_preferred();
  std::wstring wPath = path.wstring();
  wPath.push_back(L'\0');
  HINSTANCE hdll = LoadLibraryW(wPath.data());
  if (!hdll) return nullptr;
  newPlugin = reinterpret_cast<decltype(newPlugin)>(GetProcAddress(hdll, "newPlugin"));
  if (!newPlugin) {
    FreeLibrary(hdll);
  }
  auto plugin = newPlugin((*m_Settings)[pluginName]);      // nice
  m_PluginPath[plugin] = hdll;
  return plugin;
}

void PluginMgr::FreePlugin(PluginObject* plugin)
{
  delete plugin;
  auto& hdll = m_PluginPath[plugin];
  FreeLibrary(reinterpret_cast<HINSTANCE>(hdll));
  hdll = nullptr;
}
