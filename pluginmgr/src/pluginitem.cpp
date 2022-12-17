#include <pluginitem.h>
#include <pluginobject.h>
#include <pluginmgr.h>
#include <plugincenter.h>
#include <httplib.h>
#include <yjson.h>
#include <neoapp.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

#include <filesystem>
#include <format>

namespace fs = std::filesystem;

PluginItem::PluginItem(std::u8string pluginName, const YJson& data, QWidget* parent)
  : QWidget(parent)
  , m_MainLayout(new QVBoxLayout(this))
  , m_SubLayout(new QHBoxLayout)
  , m_BtnInstall(new QPushButton("安装", this))
  , m_BtnUninstall(new QPushButton("卸载", this))
  , m_BtnUpgrade(new QPushButton("更新", this))
  , m_ChkEnable(new QCheckBox("激活", this))
  , m_PluginName(pluginName)
  , m_Data(data)
{
  InitStatus();
  InitLayout();
}

PluginItem::~PluginItem()
{
}

void PluginItem::InitLayout()
{
  m_MainLayout->addLayout(m_SubLayout);

  auto label = new QLabel(
      "<h3>" + PluginObject::Utf82QString(m_Data[u8"FriendlyName"].getValueString()) + "</h3>", this);
  m_SubLayout->addWidget(label);

  auto& jsVersion = m_Data[u8"Version"];
  auto version = std::format(L"版本：{}.{}.{}",
      jsVersion[0].getValueInt(),
      jsVersion[1].getValueInt(),
      jsVersion[2].getValueInt()
  );
  label = new QLabel(QString::fromStdWString(version), this);
  m_SubLayout->addWidget(label);

  label = new QLabel("作者：" + 
      PluginObject::Utf82QString(m_Data[u8"Author"].getValueString()),
      this);
  m_SubLayout->addWidget(label);

  connect(m_BtnInstall, &QPushButton::clicked, this, std::bind(&PluginItem::Install, this));
  m_SubLayout->addWidget(m_BtnInstall);

  connect(m_BtnUpgrade, &QPushButton::clicked, this, std::bind(&PluginItem::Upgrade, this));
  m_SubLayout->addWidget(m_BtnUpgrade);

  connect(m_BtnUninstall, &QPushButton::clicked, this, std::bind(&PluginItem::Uninstall, this));
  m_SubLayout->addWidget(m_BtnUninstall);

  m_ChkEnable->setChecked(m_Enabled);
  connect(m_ChkEnable, &QCheckBox::clicked, this, [this](bool on){
    bool const ok = mgr->LoadPlugin(m_PluginName, on);
    if (!ok) m_ChkEnable->toggle();
  });
  m_SubLayout->addWidget(m_ChkEnable);

  label = new QLabel("简介：" + 
      PluginObject::Utf82QString(m_Data[u8"Description"].getValueString()), this);
  m_MainLayout->addWidget(label);

  UpdateEnabled();
}


/*
 *
  "neospeedboxplg": {
    "Enabled": true,
    "FriendlyName": "网速悬浮",
    "Description": "实时显示网速和内存占用",
    "Author": "yjmthu",
    "Version": [0, 0, 1]
  },
 *
 */

bool PluginItem::DownloadPlugin()
{
  if (!HttpLib::IsOnline()) {
    glb->glbShowMsgbox(u8"失败", u8"请检查网络连接！");
    return false;
  }
  bool result = true;
  QDialog dialog(this);
  dialog.setWindowTitle("正在下载，请稍等");

  QTimer::singleShot(100, this, [this, &result, &dialog]() {
    HttpLib clt(PluginCenter::GetRawUrl(u8"plugins/" + m_PluginName + u8"/manifest.json"));
    auto res = clt.Get();
    if (res->status != 200) {
      glb->glbShowMsgbox(u8"失败", u8"下载清单失败！");
      dialog.close();
      result = false;
      return;
    }

    const YJson data(res->body.begin(), res->body.end());
    for (auto& [dir, info]: data.getObject()) {
      if (!fs::exists(dir)) {
        fs::create_directories(dir);
      }
      fs::path path = dir;
      for (auto& subdir: info[u8"dirs"].getArray()) {
        auto subpath = path / subdir.getValueString();
        if (!fs::exists(subpath)) {
          fs::create_directory(subpath);
        }
      }
      for (auto& file: info[u8"files"].getArray()) {
        auto subpath = path / file.getValueString();
        clt.SetUrl<char8_t>(PluginCenter::GetRawUrl(dir + u8"/" + file.getValueString()));
        res = clt.Get(subpath);
        if (res->status != 200) {
          glb->glbShowMsgbox(u8"失败", u8"下载插件失败！");
          result = false;
          break;
        }
      }
    }
    dialog.close();
  });
  dialog.exec();
  return result;
}

void PluginItem::InitStatus()
{
  auto& data = mgr->GetPluginsInfo();
  auto iter = data.find(m_PluginName);
  m_Installed = iter != data.endO();
  if (!m_Installed) {
    m_CanUpdate = m_Enabled = false;
    return;
  }
  const auto& jsVersion = m_Data[u8"Version"];
  const auto& curVersion = iter->second[u8"Version"];
  m_CanUpdate = curVersion[0].getValueInt() < jsVersion[0].getValueInt() &&
    curVersion[1].getValueInt() < jsVersion[1].getValueInt() &&
    curVersion[2].getValueInt() < jsVersion[2].getValueInt();
  m_Enabled = mgr->GetPluginsInfo()[m_PluginName][u8"Enabled"].isTrue();
}

void PluginItem::UpdateEnabled()
{
  m_BtnInstall->setEnabled(!m_Installed);
  m_BtnUpgrade->setEnabled(m_CanUpdate);
  m_BtnUninstall->setEnabled(m_Installed);
  m_ChkEnable->setEnabled(m_Installed);
}

void PluginItem::Install()
{
  if (!DownloadPlugin()) {
    return;
  }
  
  if (!mgr->InstallPlugin(m_PluginName, m_Data)) {
    return;
  }

  m_Installed = true;
  m_CanUpdate = false;
  m_Enabled = false;
  UpdateEnabled();

  glb->glbShowMsg("安装成功！");
}

void PluginItem::Uninstall()
{
  if (!mgr->UnInstallPlugin(m_PluginName)) {
    return;
  }
  fs::remove_all(u8"plugins/" + m_PluginName);

  m_Installed = false;
  m_CanUpdate = false;
  m_Enabled = false;
  m_ChkEnable->setChecked(false);
  UpdateEnabled();

  glb->glbShowMsg("卸载成功！");
}

void PluginItem::Upgrade()
{
  if (!mgr->UnInstallPlugin(m_PluginName)) {
    return;
  }
  fs::remove_all(u8"plugins/" + m_PluginName);

  if (!DownloadPlugin()) {
    return;
  }
  
  if (!mgr->InstallPlugin(m_PluginName, m_Data)) {
    return;
  }

  m_Installed = true;
  m_CanUpdate = false;
  m_Enabled = false;
  UpdateEnabled();

  glb->glbShowMsg("升级成功！");
}

