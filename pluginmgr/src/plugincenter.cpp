#include <plugincenter.h>
#include <pluginitem.h>
#include <httplib.h>
#include <yjson.h>
#include <neoapp.h>

#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace std::literals;

PluginCenter::PluginCenter(YJson& setting):
  QDialog(nullptr),
  m_MainLayout(new QVBoxLayout(this)),
  m_ListWidget(new QListWidget(this)),
  m_Setting(setting)
{
  setWindowTitle("Neobox 插件");
  setMinimumWidth(600);
  setStyleSheet(glb->glbGetMenu()->styleSheet());
  m_MainLayout->addWidget(m_ListWidget);
  m_ListWidget->setResizeMode(QListView::Adjust);
  GetPluginData();
}

bool PluginCenter::GetPluginData()
{
  if (!HttpLib::IsOnline()) return false;

  HttpLib clt(GetRawUrl(u8"plugins.json"s));
  auto res = clt.Get();

  if (res->status != 200) {
    return false;
  }

  // https://gitlab.com/yjmthu1/neoboxplg/-/raw/main/plugins/neohotkeyplg/manifest.json
  const YJson data(res->body.begin(), res->body.end());

  for (auto& [name, info]: data[u8"Plugins"].getObject()) {
    auto const item = new QListWidgetItem;
    item->setSizeHint(QSize(400, 70));
    m_ListWidget->addItem(item);
    m_ListWidget->setItemWidget(item, new PluginItem(name, info, m_ListWidget));
  }

  return true;
}

PluginCenter::~PluginCenter()
{
}

std::u8string PluginCenter::GetRawUrl(const std::u8string& path)
{
  return u8"https://gitlab.com/yjmthu1/neoboxplg/-/raw/main/"s + path;
}

