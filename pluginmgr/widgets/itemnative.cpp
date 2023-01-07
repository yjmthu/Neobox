#include "itemnative.hpp"
#include "plugincenter.hpp"
#include "tabnative.hpp"
#include "tabonline.hpp"

#include <pluginobject.h>
#include <pluginmgr.h>
#include <yjson.h>
#include <glbobject.h>

#include <QListWidget>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>


ItemNative::ItemNative(std::u8string_view pluginName, const YJson& data, QWidget* parent)
  : ItemBase(pluginName, data, parent)
  , m_BtnUninstall(new QPushButton("卸载", this))
  , m_BtnUpgrade(new QPushButton("更新", this))
  , m_ChkEnable(new QCheckBox("激活", this))
{
  m_ChkEnable->setChecked(data[u8"Enabled"].isTrue());
  InitLayout();
  InitConnect();
}

ItemNative::~ItemNative()
{
  // glb->glbShowMsg("delete native item");
}

void ItemNative::InitLayout()
{
  m_MainLayout->addSpacing(15);
  m_LabelOldVersion = new QLabel(this);
  SetVersionLabel(L"当前版本", m_PluginOldVersion, m_LabelOldVersion);
  m_MainLayout->addWidget(m_LabelOldVersion);

  m_MainLayout->addSpacing(15);
  m_LabelNewVersion = new QLabel(this);
  SetVersionLabel(L"最新版本", m_PluginNewVersion, m_LabelNewVersion);
  m_MainLayout->addWidget(m_LabelNewVersion);
  m_LabelNewVersion->setVisible(false);

  m_MainLayout->addStretch();
  // connect(m_BtnInstall, &QPushButton::clicked, this, std::bind(&ItemNative::Install, this));
  // m_SubLayout->addWidget(m_BtnInstall);
  
  m_BtnUpgrade->setEnabled(false);

  m_MainLayout->addWidget(m_BtnUninstall);
  m_MainLayout->addWidget(m_BtnUpgrade);
  m_MainLayout->addWidget(m_ChkEnable);
}

void ItemNative::UpdateStatus(const YJson& pluginsInfo)
{
  auto const& info = pluginsInfo.find(m_PluginName);
  if (info == pluginsInfo.endO()) return;
  GetVersion(m_PluginNewVersion, info->second[u8"Version"]);
  SetVersionLabel(L"最新版本", m_PluginNewVersion, m_LabelNewVersion);
  m_LabelNewVersion->setVisible(true);

  m_BtnUpgrade->setEnabled(m_PluginOldVersion < m_PluginNewVersion);
}

void ItemNative::InitConnect()
{
  connect(m_BtnUpgrade, &QPushButton::clicked, this, &ItemBase::PluginUpgrade);
  m_MainLayout->addWidget(m_BtnUpgrade);

  connect(m_BtnUninstall, &QPushButton::clicked, this, &ItemBase::PluginUninstall);
  m_MainLayout->addWidget(m_BtnUninstall);

  connect(m_ChkEnable, &QCheckBox::clicked, this, [this](bool on){
    if (!mgr->TooglePlugin(m_PluginName, on)) {
      m_ChkEnable->toggle();
    }
  });
}

void ItemNative::SetUpdated()
{
  m_ChkEnable->setChecked(mgr->IsPluginEnabled(m_PluginName));
  m_BtnUpgrade->setEnabled(false);
  m_LabelOldVersion->setText(m_LabelNewVersion->text());
  m_PluginOldVersion = m_PluginNewVersion;
}

void ItemNative::DoFinished(FinishedType type, bool ok)
{
  if (type == FinishedType::Upgrade) {
    if (ok) {
      SetUpdated();
      PluginCenter::m_Instance->m_TabOnline->UpdateItem(m_PluginName, true);
    }
  } else if (type == FinishedType::Uninstall) {
    if (ok) {
      // 先
      PluginCenter::m_Instance->m_TabOnline->UpdateItem(m_PluginName, false);
      // 后
      auto const lst = PluginCenter::m_Instance->m_TabNative->m_ListWidget;
      for (int i=0; i!=lst->count(); ++i) {
        auto item = lst->item(i);
        auto w = lst->itemWidget(item);
        if (w == this) {
          delete this;
          delete item;
        }
      }
    }
  }
}

void ItemNative::GetContent(YJson& data) const
{
  data.append(YJson::O {
    {u8"Enabled", m_ChkEnable->isChecked()},
    {u8"FriendlyName", m_PluginFriendlyName},
    {u8"Description", m_PluginDescription},
    {u8"Author", m_PluginAuthor},
    {u8"Version", YJson::A {
      static_cast<double>(m_PluginOldVersion[0]),
      static_cast<double>(m_PluginOldVersion[1]),
      static_cast<double>(m_PluginOldVersion[2]),
    }}
  }, m_PluginName);
  data.toFile("Hello, world");
}
