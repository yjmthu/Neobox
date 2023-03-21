#include "tabonline.hpp"
#include "plugincenter.hpp"
#include "itemonline.hpp"

#include <pluginobject.h>
#include <pluginmgr.h>
#include <yjson.h>

#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QScrollBar>

TabOnline::TabOnline(PluginCenter* center)
  : QWidget(center)
  , m_PluginCenter(*center)
  , m_MainLayout(new QVBoxLayout(this))
  // , m_ControlLayout(new QHBoxLayout)
  , m_ListWidget(new QListWidget(this))
{
  InitLayout();
  // InitControls();
  InitPlugins();
}

TabOnline::~TabOnline()
{
}

void TabOnline::UpdateItem(std::u8string_view pluginName, bool isUpdate)
{
  for (int i=0; i!=m_ListWidget->count(); ++i) {
    auto w = qobject_cast<ItemOnline*>(m_ListWidget->itemWidget(m_ListWidget->item(i)));
    if (w->m_PluginName == pluginName) {
      if (isUpdate) {
        w->SetUpdated();
      } else {
        w->SetUninstalled();
      }
      break;
    }
  }
}

void TabOnline::UpdatePlugins()
{
  if (!m_PluginCenter.UpdatePluginData()) {
    mgr->ShowMsg("下载插件信息失败！");
    return;
  }

  if (m_ListWidget->count() == 0)
    InitPlugins();
}

void TabOnline::InitPlugins()
{
  if (!m_PluginCenter.m_PluginData)
    return;

  for (const auto& [name, info]: m_PluginCenter.m_PluginData->find(u8"Plugins")->second.getObject()) {
    auto const item = new QListWidgetItem;
    item->setSizeHint(QSize(300, 70));
    m_ListWidget->addItem(item);
    m_ListWidget->setItemWidget(item, new ItemOnline(name, info, m_ListWidget));
  }
}

void TabOnline::InitLayout()
{
  auto const background = new QWidget(this);
  background->setObjectName("whiteBackground");
  m_MainLayout->setContentsMargins(0,0,0,0);
  m_MainLayout->addWidget(background);
  auto const layout = new QHBoxLayout(background);
  layout->addWidget(m_ListWidget);

  m_ListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_PluginCenter.AddScrollBar(m_ListWidget->verticalScrollBar());
  // m_MainLayout->addLayout(m_ControlLayout);
}
