#include <bingapiex.h>
#include <pluginobject.h>
#include <yjson.h>
#include <pluginmgr.h>

#include <QInputDialog>
#include <QDesktopServices>
#include <QFileDialog>

#include <filesystem>
#include <chrono>

namespace chrono = std::chrono;
using namespace std::literals;

BingApiExMenu::BingApiExMenu(YJson data, MenuBase* parent, Callback callback)
  : WallBaseEx(callback, std::move(data), parent)
{
  LoadSettingMenu();
}

BingApiExMenu::~BingApiExMenu()
{
  //
}

void BingApiExMenu::LoadSettingMenu()
{
  connect(addAction("存储路径"), &QAction::triggered, this, [this]() {
    auto& u8CurDir = m_Data[u8"directory"].getValueString();
    auto u8NewDir = GetExistingDirectory(
      QStringLiteral("选择存储壁纸的文件夹"), u8CurDir
    );
    
    if (!u8NewDir) {
      mgr->ShowMsg("取消成功");
    } else {
      u8CurDir.swap(*u8NewDir);
      mgr->ShowMsg("设置成功");
      SaveSettings();
    }
  });

  connect(addAction("名称格式"), &QAction::triggered, this, [this]() {
    auto& u8ImgFmtOld = m_Data[u8"name-format"].getValueString();
    auto u8ImgFmtNew = GetNewU8String("图片名称", "输入名称格式", u8ImgFmtOld);
    if (!u8ImgFmtNew) {
      mgr->ShowMsg("取消成功");
    } else {
      u8ImgFmtOld.swap(*u8ImgFmtNew);
      SaveSettings();
    }
  });

  connect(addAction("地理位置"), &QAction::triggered, this, [this]() {
    auto& u8MktOld = m_Data[u8"region"].getValueString();
    auto u8MktNew =  GetNewU8String("图片地区", "输入图片所在地区", u8MktOld);
    if (u8MktNew) {
      u8MktOld.swap(*u8MktNew);
      SaveSettings();
      mgr->ShowMsg("设置成功");
    } else {
      mgr->ShowMsg("取消成功");
    }
  });
  auto const action = addAction("自动下载");
  action->setCheckable(true);
  action->setChecked(m_Data[u8"auto-download"].isTrue());
  connect(action, &QAction::triggered, this, [this](bool checked) {
    m_Data[u8"auto-download"] = checked;
    SaveSettings();
  });

  connect(addAction("关于壁纸"), &QAction::triggered, this, [this]() {
#ifdef _WIN32
    const auto time = chrono::current_zone()->to_local(chrono::system_clock::now() - 24h);
    std::string curDate = std::format("&filters=HpDate:\"{0:%Y%m%d}_1600\"", time);
#else
    time_t timep;
    time(&timep);

    auto const p = gmtime(&timep);
    std::string curDate = std::format("&filters=HpDate:\"{:04d}{:02d}{:02d}_1600\"", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday);
#endif
    auto const& copyrightlink = m_Data[u8"copyrightlink"];
    if (!copyrightlink.isString()) {
      mgr->ShowMsg("找不到当前图片信息！");
      return;
    }
    std::u8string link = copyrightlink.getValueString();
    if (link.empty()) {
      mgr->ShowMsg("找不到当前图片信息！");
      return;
    }
    link.append(curDate.cbegin(), curDate.cend());
    QDesktopServices::openUrl(QString::fromUtf8(link.data(), link.size()));
  });
}
