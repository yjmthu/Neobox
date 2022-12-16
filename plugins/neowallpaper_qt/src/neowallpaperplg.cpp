#include <wallhavenex.h>
#include <bingapiex.h>
#include <directapiex.h>
#include <nativeex.h>
#include <neowallpaperplg.h>
#include <wallpaper.h>
#include <yjson.h>
#include <neoapp.h>

#include <QString>
#include <QInputDialog>
#include <QDesktopServices>
#include <QFileDialog>
#include <QActionGroup>
#include <QMenu>
#include <QDropEvent>
#include <QMimeData>

#include <windows.h>

#include <ranges>

#define CLASS_NAME NeoWallpaperPlg
#include <pluginexport.cpp>

NeoWallpaperPlg::NeoWallpaperPlg(YJson& settings):
  PluginObject(InitSettings(settings), u8"neowallpaperplg", u8"壁纸引擎"),
  m_Wallpaper(new Wallpaper(m_Settings, std::bind(&PluginMgr::SaveSettings, mgr)))
{
  QDir dir;
  if (!dir.exists("junk"))
    dir.mkdir("junk");
  InitFunctionMap();
}

NeoWallpaperPlg::~NeoWallpaperPlg()
{
  delete m_MainMenuAction;
  // delete m_MainMenu;
  delete m_Wallpaper;
}

void NeoWallpaperPlg::InitFunctionMap()
{
  m_PluginMethod = {
    {u8"setCurrentDir",
      {u8"设置位置", u8"设置当前壁纸来源存储位置",
        [this](PluginEvent, void*) {
        QString current =
            QString::fromStdU16String(m_Wallpaper->GetImageDir().u16string());
        current = QFileDialog::getExistingDirectory(glb->glbGetMenu(), "选择壁纸文件夹", current);
return ;
                                
        if (current.isEmpty() || !QDir().exists(current)) {
          glb->glbShowMsg("取消设置成功！");
          return;
        }
        m_Wallpaper->SetCurDir(current.toStdWString());
        glb->glbShowMsg("设置壁纸存放位置成功！");
      }, PluginEvent::Void},
    },
    {u8"setTimeInterval",
      {u8"时间间隔", u8"设置更换壁纸的时间间隔", [this](PluginEvent, void*){
        int iNewTime =
            QInputDialog::getInt(glb->glbGetMenu(), "输入时间间隔", "时间间隔（分钟）：", m_Wallpaper->GetTimeInterval(), 5);
        if (iNewTime < 5) {
          glb->glbShowMsg("设置时间间隔失败！");
          return;
        }
        m_Wallpaper->SetTimeInterval(iNewTime);
        glb->glbShowMsg("设置时间间隔成功！");
      }, PluginEvent::Void}
    },
    {u8"openCurrentDir",
      { u8"打开位置", u8"打开当前壁纸位置", [this](PluginEvent, void*) {
        std::wstring args = L"/select, " + m_Wallpaper->GetCurIamge().wstring();
        ShellExecuteW(nullptr, L"open", L"explorer", args.c_str(), NULL, SW_SHOWNORMAL);
      }, PluginEvent::Void
    }},
    {u8"cleanRubish",
      { u8"清理垃圾", u8"删除垃圾箱内壁纸", [this](PluginEvent, void*){
        m_Wallpaper->ClearJunk();
        glb->glbShowMsg("清除壁纸垃圾成功！");
      }, PluginEvent::Void
    }},
    {u8"setAutoChange",
      {u8"自动更换", u8"设置是否按照时间间隔自动切换壁纸", [this](PluginEvent event, void* data) {
        if (event == PluginEvent::Bool) {
          m_Wallpaper->SetAutoChange(*reinterpret_cast<bool*>(data));
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool*>(data) = m_Wallpaper->GetAutoChange();
        }
      },
      PluginEvent::Bool }
    },
    {u8"setFirstChange",
      {u8"首次更换", u8"设置是否在插件启动时更换一次壁纸", [this](PluginEvent event, void* data) {
        if (event == PluginEvent::Bool) {
          m_Wallpaper->SetFirstChange(*reinterpret_cast<bool*>(data));
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool*>(data) = m_Wallpaper->GetFirstChange();
        }
      }, PluginEvent::Bool },
    }
  };

  m_Following.push_back({u8"neospeedboxplg", [this](PluginEvent event, void* data){
    if (event == PluginEvent::Drop) {
      const auto mimeData = reinterpret_cast<QDropEvent*>(data)->mimeData();
      if (!mimeData->hasUrls()) return;
      auto urls = mimeData->urls();
      auto picUrlsView =
      urls | std::views::filter([](const QUrl& i) { return i.isValid() && (i.isLocalFile() || i.scheme().startsWith("http")); })
      | std::views::transform([](const QUrl& i) {
      auto const data = i.isLocalFile() ? i.toLocalFile() : i.toString();
      return PluginObject::QString2Utf8(data); });
      m_Wallpaper->SetDropFile(std::vector<std::u8string>(picUrlsView.begin(), picUrlsView.end()));
    }
  }});
}

QAction* NeoWallpaperPlg::InitMenuAction()
{
  m_MoreSettingsAction = new QAction("更多设置", m_MainMenu);
  LoadWallpaperTypeMenu(m_MainMenu);
  this->PluginObject::InitMenuAction();
  m_MainMenu->addAction(m_MoreSettingsAction);
  LoadWallpaperExMenu(m_MainMenu);
  LoadMainMenuAction();

  return m_MainMenuAction;
}

void NeoWallpaperPlg::LoadMainMenuAction()
{
  m_MainMenuAction = new QAction("壁纸切换");
  auto const menu = new QMenu(m_MainMenu);
  menu->setAttribute(Qt::WA_TranslucentBackground, true);
  menu->setToolTipsVisible(true);
  m_MainMenuAction->setMenu(menu);

  std::vector<std::pair<std::u8string, FunctionInfo>> temp = {
    { u8"prev",
      {u8"上一张图", u8"切换到上一张壁纸", [this](PluginEvent, void*) {
          m_Wallpaper->SetSlot(-1);
        }, PluginEvent::Void }
    },
    {u8"next",
      {u8"下一张图", u8"切换到下一张壁纸", [this](PluginEvent, void*) {
          m_Wallpaper->SetSlot(1);
        }, PluginEvent::Void
      },
    },
    {u8"dislike",
      {u8"不看此图", u8"把这张图移动到垃圾桶", [this](PluginEvent, void*) {
          m_Wallpaper->SetSlot(0);
        }, PluginEvent::Void
      },
    },
    {u8"undoDislike",
      {u8"撤销删除", u8"撤销上次的删除操作",
       [this](PluginEvent, void*){
          m_Wallpaper->UndoDelete();
        }, PluginEvent::Void
      },
    },
    {u8"collect",
      {u8"收藏图片", u8"把当前壁纸复制到收藏夹", [this](PluginEvent, void*) {
        m_Wallpaper->Wallpaper::SetFavorite();
        glb->glbShowMsg("收藏壁纸成功！");
      }, PluginEvent::Void},
    },
    {u8"undoCollect",
      {u8"撤销收藏", u8"如果当前壁纸在收藏夹内，则将其移出", [this](PluginEvent, void*) {
          m_Wallpaper->UnSetFavorite();
          glb->glbShowMsg("撤销收藏壁纸成功！");
        }, PluginEvent::Void
      },
    },
  };

  for (auto& [name, info]: temp) {
    auto const action = menu->addAction(
          Utf82QString(info.friendlyName));
    action->setToolTip(PluginObject::Utf82QString(info.description));
    QObject::connect(action, &QAction::triggered, menu, std::bind(info.function, PluginEvent::Void, nullptr));
    m_PluginMethod[name] = std::move(info);
  }
}

YJson& NeoWallpaperPlg::InitSettings(YJson& settings)
{
  if (settings.isObject()) return settings;
  return settings = YJson::O {
    {u8"AutoChange", false},
    {u8"FirstChange", false},
    {u8"TimeInterval", 30},
    {u8"ImageType", 0},
    {u8"DropToCurDir", true},
    {u8"DropDir", (fs::current_path() / "junk").u8string()},
    {u8"DropImgUseUrlName", true},
    {u8"DropINameFmt", u8"drop-image {1} {0:%Y-%m-%d}"},
  };
  // we may not need to call SaveSettings;
}

void NeoWallpaperPlg::LoadWallpaperTypeMenu(QMenu* pluginMenu)
{
  static const char* sources[12] = {
    "壁纸天堂", "来自https://wallhaven.cc的壁纸",
    "必应壁纸", "来自https://www.bing.com的壁纸",
    "直链壁纸", "下载并设置链接中的壁纸，例如https://source.unsplash.com/random",
    "本地壁纸", "将本地文件夹内的图片作为壁纸来源",
    "脚本壁纸", "使用脚本运行输出的本地图片路径作为壁纸",
    "收藏壁纸", "来自用户主动收藏的壁纸",
  };
  auto mainAction = pluginMenu->addAction("壁纸来源");
  mainAction->setToolTip("设置壁纸来源");
  m_WallpaperTypesMenu = new QMenu(pluginMenu);
  m_WallpaperTypesMenu->setAttribute(Qt::WA_TranslucentBackground, true);
  m_WallpaperTypesMenu->setToolTipsVisible(true);
  mainAction->setMenu(m_WallpaperTypesMenu);
  m_WallpaperTypesGroup = new QActionGroup(m_WallpaperTypesMenu);

  const int curType = m_Wallpaper->GetImageType();

  for (size_t i = 0; i != 6; ++i) {
    QAction* action = m_WallpaperTypesMenu->addAction(sources[2*i]);
    action->setToolTip(sources[2*i+1]);
    action->setCheckable(true);
    m_WallpaperTypesGroup->addAction(action);
    QObject::connect(action, &QAction::triggered, m_WallpaperTypesMenu, [this, i, pluginMenu]() {
      const int oldType = m_Wallpaper->GetImageType();
      if (m_Wallpaper->SetImageType(i)) {
        LoadWallpaperExMenu(pluginMenu);
        glb->glbShowMsg("设置壁纸来源成功！");
      } else {
        m_WallpaperTypesGroup->actions().at(oldType)->setChecked(true);
        glb->glbShowMsg("当前正忙，设置失败！");
      }
    });
    action->setChecked(i == curType);   // very nice
  } 
}

void NeoWallpaperPlg::LoadWallpaperExMenu(QMenu* parent)
{
  static QMenu* (NeoWallpaperPlg::*const m_MenuLoaders[6])(QMenu*) {
    &NeoWallpaperPlg::LoadWallavenMenu,
    &NeoWallpaperPlg::LoadBingApiMenu,
    &NeoWallpaperPlg::LoadDirectApiMenu,
    &NeoWallpaperPlg::LoadNativeMenu,
    &NeoWallpaperPlg::LoadScriptMenu,
    &NeoWallpaperPlg::LoadFavoriteMenu,
  };
  delete m_MoreSettingsAction->menu();
  auto const menu = (this->*m_MenuLoaders[m_Wallpaper->GetImageType()])(parent);
  menu->setAttribute(Qt::WA_TranslucentBackground, true);
  menu->setToolTipsVisible(true);
  m_MoreSettingsAction->setMenu(menu);
}

QMenu* NeoWallpaperPlg::LoadWallavenMenu(QMenu* parent)
{
  return new WallhavenExMenu(
    m_Wallpaper->m_Wallpaper->m_Setting,
    parent,
    std::bind(&WallBase::SetJson, m_Wallpaper->m_Wallpaper, std::placeholders::_1),
    std::bind(&Wallpaper::GetCurIamge, m_Wallpaper)
  );
}

QMenu* NeoWallpaperPlg::LoadBingApiMenu(QMenu* parent)
{
  return new BingApiExMenu(
    m_Wallpaper->m_Wallpaper->m_Setting,
    parent,
    std::bind(&WallBase::SetJson, m_Wallpaper->m_Wallpaper, std::placeholders::_1)
  );
}

QMenu* NeoWallpaperPlg::LoadDirectApiMenu(QMenu* parent)
{
  return new DirectApiExMenu(
    m_Wallpaper->m_Wallpaper->m_Setting,
    parent,
    std::bind(&WallBase::SetJson, m_Wallpaper->m_Wallpaper, std::placeholders::_1)
  );
}

QMenu* NeoWallpaperPlg::LoadNativeMenu(QMenu* parent)
{
  return new NativeExMenu(
    m_Wallpaper->m_Wallpaper->m_Setting,
    parent,
    std::bind(&WallBase::SetJson, m_Wallpaper->m_Wallpaper, std::placeholders::_1)
  );
}

QMenu* NeoWallpaperPlg::LoadScriptMenu(QMenu* parent)
{
  return new NativeExMenu(
    m_Wallpaper->m_Wallpaper->m_Setting,
    parent,
    std::bind(&WallBase::SetJson, m_Wallpaper->m_Wallpaper, std::placeholders::_1)
  );
}

QMenu* NeoWallpaperPlg::LoadFavoriteMenu(QMenu* parent)
{
  return new QMenu(parent);
}
