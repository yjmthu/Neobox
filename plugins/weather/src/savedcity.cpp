#include <savedcity.hpp>
#include <citylist.hpp>
#include <savedcityitem.hpp>

#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QListWidgetItem>
#include <QFile>


SavedCity::SavedCity(QWidget* parent, WeatherCfg& cfg)
  : WidgetBase(parent)
  , m_Config(cfg)
  , m_CenterWidget(new QWidget(this))
  , m_List(new QListWidget(m_CenterWidget))
{
  SetStyleSheet();
  setAttribute(Qt::WA_DeleteOnClose);
  SetupCities();
  SetupUi();
}

SavedCity::~SavedCity()
{
  if (m_DataChanged) {
    m_Config.SaveData();
  }
  emit Finished(true);
}

void SavedCity::SetupUi()
{
  setAttribute(Qt::WA_TranslucentBackground);
  setWindowFlag(Qt::Tool);

  // setFixedSize(400, 400);

  auto layout = new QVBoxLayout(this);
  layout->addWidget(m_CenterWidget);
  m_CenterWidget->setObjectName("Weather");

  auto mainLayout = new QVBoxLayout(m_CenterWidget);
  mainLayout->setContentsMargins(11, 30, 11, 11);

  AddTitle("城市列表");
  AddCloseButton();
  SetShadowAround(m_CenterWidget);

  mainLayout->addWidget(m_List);
  AddScrollBar(m_List->verticalScrollBar());
  m_List->setFixedSize(360, 400);

  m_List->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void SavedCity::SetStyleSheet()
{
  QFile file(":/styles/CityStyle.qss");
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  setStyleSheet(file.readAll());

  file.close();
}

void SavedCity::SetupCities()
{
  auto list = m_Config.GetCityList();
  m_CurId = m_Config.GetCity();

  for (auto& [id, item]: list.getObject()) {
    auto i = new QListWidgetItem;
    i->setSizeHint(QSize(350, 50));
    auto w = new SavedCityItem(this, i, m_CurId, id, item);
    // w->setFixedSize(300, 40);
    if (id == m_CurId) m_LastItem = w;
    m_List->addItem(i);
    m_List->setItemWidget(i, w);
    connect(w, &SavedCityItem::Clicked, this, &SavedCity::ChangeCurrent);
    connect(w, &SavedCityItem::Deleted, this, &SavedCity::DeleteItem);
  }

  m_List->setCurrentItem(m_LastItem->m_Item);

  connect(m_List, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
    auto w = qobject_cast<SavedCityItem*>(m_List->itemWidget(item));
    if (m_CurId != w->m_Info.id) {
      ChangeCurrent(w);
    }
  });
}

void SavedCity::ChangeCurrent(SavedCityItem* item)
{
  m_LastItem->SetChecked(false);

  m_LastItem = item;
  m_LastItem->SetChecked(true);

  m_CurId = item->m_Info.id;
  item->m_Delete->hide();
  m_Config.SetCity(m_CurId, false);
  m_DataChanged = true;

  m_List->blockSignals(true);
  m_List->setCurrentItem(item->m_Item);
  m_List->blockSignals(false);
  emit Finished(false);
}

void SavedCity::DeleteItem(SavedCityItem* widget)
{
  m_CurId = widget->m_Info.id;
  auto const item = widget->m_Item;
  delete widget;
  delete item;

  SaveData();
}

void SavedCity::SaveData()
{
  YJson json(YJson::Object);
  for (int i=0; i!=m_List->count(); ++i) {
    auto item = m_List->item(i);
    auto w = qobject_cast<SavedCityItem*>(m_List->itemWidget(item));
    json[w->m_Info.id] = w->m_Info.DumpJSON();
  }
  m_Config.SetCityList(std::move(json), false);
  m_DataChanged = true;
}

void SavedCity::Update() {
  m_List->clear();

  SetupCities();
}