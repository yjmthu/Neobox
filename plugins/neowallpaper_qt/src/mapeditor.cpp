#include <mapeditor.h>
#include <pluginobject.h>
#include <yjson.h>

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>

MapEditor::MapEditor(QString title, YJson& data, const std::function<void()> callback):
  QWidget(nullptr),
  m_Data(data),
  m_CallBack(callback),
  m_Table(new QTableWidget(data.sizeO(), 2, this))
{
  setWindowTitle(title);
  setAttribute(Qt::WA_DeleteOnClose);
  SetBaseLayout();
}

MapEditor::~MapEditor()
{
  SaveData();
  m_CallBack();
}

void MapEditor::SetBaseLayout()
{
  auto const pVLayout = new QVBoxLayout(this);
  auto const pHLayout = new QHBoxLayout;
  std::array arButtons = {
      new QPushButton("添加", this), new QPushButton("删除", this),
      new QPushButton("确认", this), new QPushButton("取消", this)};
  std::for_each(arButtons.begin(), arButtons.end(),
                std::bind(&QHBoxLayout::addWidget, pHLayout,
                          std::placeholders::_1, 0, Qt::Alignment()));
  pVLayout->addLayout(pHLayout);

  for (int i = 0; const auto& [key, value] : m_Data.getObject()) {
    std::u8string_view value_view = value.getValueString();
    m_Table->setItem(
        i, 0,
        new QTableWidgetItem(QString::fromUtf8(key.data(), key.size())));
    m_Table->setItem(i, 1,
                    new QTableWidgetItem(QString::fromUtf8(
                        value_view.data(), value_view.size())));
    ++i;
  }
  
  m_Table->setHorizontalHeaderLabels(
      {QStringLiteral("key"), QStringLiteral("value")});
  pVLayout->insertWidget(0, m_Table);

  connect(arButtons[0], &QPushButton::clicked, this,
      [this]() {
          if (m_Table->currentRow() == -1) {
            m_Table->insertRow(m_Table->rowCount());
          } else {
            m_Table->insertRow(m_Table->currentRow());
          }
      });
  connect(arButtons[1], &QPushButton::clicked, this,
          [this]() { m_Table->removeRow(m_Table->currentRow()); });
  connect(arButtons[2], &QPushButton::clicked, this, &MapEditor::SaveData);
  connect(arButtons[3], &QPushButton::clicked, this, &MapEditor::close);
}

void MapEditor::SaveData()
{
  YJson jsData = YJson::Object;
  for (int i = 0; i < m_Table->rowCount(); ++i) {
    auto ptr = m_Table->item(i, 0);
    if (ptr == nullptr) continue;
    auto const qsKeyArray = ptr->text();
    ptr = m_Table->item(i, 1);
    if (ptr == nullptr) continue;
    auto const qsValueArray = ptr->text();
    if (qsKeyArray.isEmpty() || qsValueArray.isEmpty())
      continue;
    jsData.append(PluginObject::QString2Utf8(qsValueArray),
                  PluginObject::QString2Utf8(qsKeyArray));
  }
  m_Data.swap(jsData);
  close();
}
