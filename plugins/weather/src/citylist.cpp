#include <citylist.hpp>
#include <yjson.h>
#include <pluginmgr.h>

#include <QLineEdit>
#include <QLabel>

CityList::CityList(QWidget* parent, QLabel& label, QLineEdit& edit)
  : QListWidget(parent)
  , m_Label(label)
  , m_Edit(edit)
{
  setFixedWidth(m_Edit.width());
  setMinimumHeight(80);
  setMaximumHeight(150);
  ConnectAll();
}

void CityList::Move()
{
  auto pos = m_Edit.geometry().bottomLeft();
  move(pos);
}

void CityList::Show()
{
  if (count()) show();
}

void CityList::ConnectAll()
{
  connect(this, &QListWidget::pressed, this, [this](const QModelIndex& index) {
    mgr->ShowMsg(QString::number(index.row()));
    auto&& text = m_Info[index.row()].GetCityName();
    m_Label.setText(QString::fromUtf8(text.data(), text.size()));
    hide();
  });
}

void CityList::SetConetnt(const YJson& data)
{
  blockSignals(true);
  clear();
  m_Info.clear();
  auto& location = data[u8"location"].getArray();
  for (auto& item: location) {
    auto& info = m_Info.emplace_back(CityInfo {
      .name = item[u8"name"].getValueString(),
      .adm1 = item[u8"adm1"].getValueString(),
      .adm2 = item[u8"adm2"].getValueString(),
      .id = item[u8"id"].getValueString(),
    });
    auto data = info.adm1 + u8" - " + info.adm2 + u8" - " + info.name;
    auto i = new QListWidgetItem(QString::fromUtf8(data.data(), data.size()));
    i->setToolTip(QString::fromUtf8(info.id.data(), info.id.size()));
    addItem(i);
  }
  blockSignals(false);
}
