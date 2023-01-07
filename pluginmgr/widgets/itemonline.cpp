#include "itemonline.hpp"
#include "plugincenter.hpp"
#include "tabnative.hpp"

#include <pluginmgr.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

ItemOnline::ItemOnline(std::u8string_view pluginName, const YJson& data, QWidget* parent)
  : ItemBase(pluginName, data, parent)
  , m_BtnContrl(new QPushButton(this))
{
  InitStatus();
  InitLayout();
  InitConnect();
}

ItemOnline::~ItemOnline()
{
  //
}

void ItemOnline::InitLayout()
{
  // m_MainLayout->addSpacing(15);
  m_LabelVersion = new QLabel(this);
  SetVersionLabel(L"版本", m_PluginNewVersion, m_LabelVersion);
  m_MainLayout->addWidget(m_LabelVersion);

  m_MainLayout->addWidget(m_BtnContrl);

  if (m_Installed) {
    if (m_PluginOldVersion < m_PluginNewVersion) {
      m_BtnContrl->setText("更新");
    } else {
      m_BtnContrl->setText("已安装");
      m_BtnContrl->setEnabled(false);
    }
  } else {
    m_BtnContrl->setText("安装");
  }
}

void ItemOnline::InitStatus()
{
  auto& info = mgr->GetPluginsInfo();
  auto iter = info.find(m_PluginName);
  m_Installed = iter != info.endO();

  if (m_Installed) {
    GetVersion(m_PluginOldVersion, iter->second.find(u8"Version")->second);
  }
}

void ItemOnline::InitConnect()
{
  if (m_Installed)
    connect(m_BtnContrl, &QPushButton::clicked, this, &ItemBase::PluginUpgrade);
  else
    connect(m_BtnContrl, &QPushButton::clicked, this, &ItemBase::PluginInstall);
}

void ItemOnline::DoFinished(FinishedType type, bool ok)
{
  if (type == FinishedType::Upgrade) {
    if (ok) {
      SetUpdated();
      PluginCenter::m_Instance->m_TabNative->UpdateItem(m_PluginName);
    }
  } else if (type == FinishedType::Install) {
    if (ok) {
      SetUpdated();
      PluginCenter::m_Instance->m_TabNative->AddItem(m_PluginName, GetContent());
    }
  }
}

void ItemOnline::SetUninstalled()
{
  m_BtnContrl->setText("安装");
  m_BtnContrl->setEnabled(true);
  m_Installed = false;
  disconnect(m_BtnContrl, nullptr, this, nullptr);
  connect(m_BtnContrl, &QPushButton::clicked, this, &ItemBase::PluginInstall);
}

void ItemOnline::SetUpdated()
{
  m_BtnContrl->setText("已安装");
  m_BtnContrl->setEnabled(false);
  m_Installed = true;
}

YJson ItemOnline::GetContent() const
{
  return YJson::O {
    {u8"Enabled", false},
    {u8"FriendlyName", m_PluginFriendlyName},
    {u8"Description", m_PluginDescription},
    {u8"Author", m_PluginAuthor},
    {u8"Version", YJson::A {
      static_cast<double>(m_PluginOldVersion[0]),
      static_cast<double>(m_PluginOldVersion[1]),
      static_cast<double>(m_PluginOldVersion[2]),
    }}
  };
}