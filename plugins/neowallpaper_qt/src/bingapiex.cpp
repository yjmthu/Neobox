#include <bingapiex.h>
#include <pluginobject.h>
#include <yjson.h>
#include <neoapp.h>

#include <QInputDialog>
#include <QDesktopServices>
#include <QFileDialog>

#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;
namespace chrono = std::chrono;
using namespace std::literals;

BingApiExMenu::BingApiExMenu(YJson& data, QMenu* parent, std::function<void(bool)> callback):
  QMenu(parent),
  m_Data(data),
  m_CallBack(callback)
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
    auto const qNewDir = QFileDialog::getExistingDirectory(
      glb->glbGetMenu(),
      QStringLiteral("选择存储壁纸的文件夹"),
      PluginObject::Utf82QString(u8CurDir)
    );
    if(qNewDir.isEmpty()) {
      glb->glbShowMsg("取消成功");
      return;
    }
    fs::path path = qNewDir.toStdU16String();
    path.make_preferred();
    
    if (path.u8string() == u8CurDir) {
      glb->glbShowMsg("取消成功");
      return;
    }

    u8CurDir = path.u8string();
    glb->glbShowMsg("设置成功");
    m_CallBack(true);
  });

  connect(addAction("名称格式"), &QAction::triggered, this, [this]() {
    auto& u8ImgFmt = m_Data[u8"name-format"].getValueString();
    QString qImgFmt = QString::fromUtf8(u8ImgFmt.data(), u8ImgFmt.size());
    const QString qImgFmtNew =
        QInputDialog::getText(this, "图片名称", "输入名称格式",QLineEdit::Normal, qImgFmt);
    std::u8string u8ImgFmtNew(PluginObject::QString2Utf8(qImgFmtNew));
    if (u8ImgFmtNew.empty() || u8ImgFmtNew == u8ImgFmt)
      return;
    u8ImgFmt.swap(u8ImgFmtNew);
    m_CallBack(true);
  });

  connect(addAction("地理位置"), &QAction::triggered, this, [this]() {
    auto& u8Mkt = m_Data[u8"region"].getValueString();
    QString qMkt = QString::fromUtf8(u8Mkt.data(), u8Mkt.size());
    const QString qMktNew =
        QInputDialog::getText(this, "图片地区", "输入图片所在地区", QLineEdit::Normal, qMkt);
    std::u8string u8MktNew(PluginObject::QString2Utf8(qMktNew));
    if (u8MktNew.empty() || u8MktNew == u8Mkt)
      return;
    u8Mkt.swap(u8MktNew);
    m_CallBack(true);
  });
  auto const action = addAction("自动下载");
  action->setCheckable(true);
  action->setChecked(m_Data[u8"auto-download"].isTrue());
  connect(action, &QAction::triggered, this, [this](bool checked) {
    m_Data[u8"auto-download"] = checked;
    m_CallBack(true);
  });

  connect(addAction("关于壁纸"), &QAction::triggered, this, [this]() {
    const auto time = chrono::current_zone()->to_local(chrono::system_clock::now() - 24h);
    std::string curDate = std::format("&filters=HpDate:\"{0:%Y%m%d}_1600\"", time);
    auto const& copyrightlink = m_Data[u8"copyrightlink"];
    if (!copyrightlink.isString()) {
      glb->glbShowMsg("找不到当前图片信息！");
      return;
    }
    std::u8string link = copyrightlink.getValueString();
    if (link.empty()) {
      glb->glbShowMsg("找不到当前图片信息！");
      return;
    }
    link.append(curDate.cbegin(), curDate.cend());
    QDesktopServices::openUrl(QString::fromUtf8(link.data(), link.size()));
  });
}
