#include "plugincenter.hpp"
#include "itemnative.hpp"
#include "tabnative.hpp"

#include <glbobject.h>
#include <pluginmgr.h>
#include <yjson.h>

#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

TabNative::TabNative(PluginCenter* center)
  : QWidget(center)
  , m_PluginCenter(*center)
  , m_MainLayout(new QVBoxLayout(this))
  , m_ControlLayout(new QHBoxLayout)
  , m_ListWidget(new QListWidget(this))
{
  InitLayout();
  InitControls();
  InitPlugins();
}

TabNative::~TabNative()
{
}

void TabNative::AddItem(std::u8string_view pluginName, const YJson& data)
{
  auto const item = new QListWidgetItem;
  item->setSizeHint(QSize(400, 70));
  m_ListWidget->addItem(item);
  m_ListWidget->setItemWidget(item, new ItemNative(pluginName, data, m_ListWidget));
}

void TabNative::UpdateItem(std::u8string_view pluginName)
{
  for (int i=0; i!=m_ListWidget->count(); ++i) {
    auto w = qobject_cast<ItemNative*>(m_ListWidget->itemWidget(m_ListWidget->item(i)));
    if (w->m_PluginName == pluginName) {
      w->SetUpdated();
      break;
    }
  }
}

void TabNative::UpdatePlugins()
{
  if (!m_PluginCenter.UpdatePluginData()) {
    glb->glbShowMsg("下载插件信息失败！");
    return;
  }

  auto& pluginsInfo = m_PluginCenter.m_PluginData->find(u8"Plugins")->second;
  for (auto i = 0; i != m_ListWidget->count(); ++i) {
    auto const w = m_ListWidget->itemWidget(m_ListWidget->item(i));
    qobject_cast<ItemNative*>(w)->UpdateStatus(pluginsInfo);
  }
}

void TabNative::UpgradeAllPlugins()
{
  if (!m_PluginCenter.UpdatePluginData()) {
    glb->glbShowMsg("下载插件信息失败！");
    return;
  }

  std::vector<ItemNative*> vec;
  auto& pluginsInfo = m_PluginCenter.m_PluginData->find(u8"Plugins")->second;
  for (auto i = 0; i != m_ListWidget->count(); ++i) {
    auto const widget = qobject_cast<ItemNative*>(m_ListWidget->itemWidget(m_ListWidget->item(i)));
    widget->UpdateStatus(pluginsInfo);
    if (widget->CanUpgrade())
      vec.push_back(widget);
  }
  if (vec.empty()) {
    glb->glbShowMsg("全部插件已是最新！");
  } else {
    std::for_each(vec.begin(), vec.end(), std::bind(&ItemNative::PluginUpgrade, std::placeholders::_1));
  }
}

void TabNative::SaveContent()
{
  YJson data(YJson::Object);    // 不能是 YJson data = YJson::Object 或 YJson data { YJson::Object }

  for (auto i = 0; i != m_ListWidget->count(); ++i) {
    auto const w = m_ListWidget->itemWidget(m_ListWidget->item(i));
    qobject_cast<const ItemNative*>(w)->GetContent(data);
  }

  mgr->UpdatePluginOrder(std::move(data));
}

void TabNative::InitControls()
{
  QPushButton* button = nullptr;

  button = new QPushButton("上移插件", this); 
  m_ControlLayout->addWidget(button);
  connect(button, &QPushButton::clicked, this, &TabNative::MoveUp);
  button = new QPushButton("下移插件", this);
  m_ControlLayout->addWidget(button);
  connect(button, &QPushButton::clicked, this, &TabNative::MoveDown);
  button = new QPushButton("保存顺序", this);
  m_ControlLayout->addWidget(button);
  connect(button, &QPushButton::clicked, this, &TabNative::SaveContent);
  button = new QPushButton("检查更新", this);
  m_ControlLayout->addWidget(button);
  connect(button, &QPushButton::clicked, this, &TabNative::UpdatePlugins);
  button = new QPushButton("全部更新", this);
  m_ControlLayout->addWidget(button);
  connect(button, &QPushButton::clicked, this, &TabNative::UpgradeAllPlugins);
}

void TabNative::InitLayout()
{
  m_MainLayout->addWidget(m_ListWidget);
  m_MainLayout->addLayout(m_ControlLayout);
}

void TabNative::InitPlugins()
{
  for (auto& [name, info]: mgr->GetPluginsInfo().getObject()) {
    auto const item = new QListWidgetItem;
    item->setSizeHint(QSize(400, 70));
    m_ListWidget->addItem(item);
    m_ListWidget->setItemWidget(item, new ItemNative(name, info, m_ListWidget));
  }
}

void TabNative::MoveUp()
{   
  if(!m_ListWidget->currentItem()) return;

  auto const curRow = m_ListWidget->currentRow();
  if(curRow <= 0) return;

  // 不能写成 { YJson::Object }，会出大问题
  YJson data(YJson::Object);
  auto const widget = qobject_cast<ItemNative*>(m_ListWidget->itemWidget(m_ListWidget->item(curRow)));
  widget->GetContent(data);
  auto& [pluginName, info] = data.frontO();

  auto item = m_ListWidget->takeItem(curRow);
  m_ListWidget->insertItem(curRow - 1, item);
  // takeItem 会自动 delete 相应的 widget，所以要重新 new 一个
  m_ListWidget->setItemWidget(item, new ItemNative(pluginName, info, m_ListWidget));
  m_ListWidget->setCurrentRow(curRow - 1);
}

void TabNative::MoveDown()
{
  if(!m_ListWidget->currentItem()) return;

  auto const curRow = m_ListWidget->currentRow();
  if(curRow + 1 >= m_ListWidget->count()) return;

  YJson data(YJson::Object);
  auto const widget = qobject_cast<ItemNative*>(m_ListWidget->itemWidget(m_ListWidget->item(curRow)));
  widget->GetContent(data);
  auto& [pluginName, info] = data.frontO();

  auto const item = m_ListWidget->takeItem(curRow);
  m_ListWidget->insertItem(curRow + 1, item);
  m_ListWidget->setItemWidget(item, new ItemNative(pluginName, info, m_ListWidget));
  m_ListWidget->setCurrentRow(curRow + 1);
}
