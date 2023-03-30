#include <listeditor.h>
#include <pluginobject.h>
#include <yjson.h>

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>

ListEditor::ListEditor(QString title, YJson data, Callback callback)
  : EditorBase(std::move(data), callback)
  , QDialog(nullptr)
  , m_List(new QListWidget(this))
{
  setWindowTitle(title);
  SetBaseLayout();
}

ListEditor::~ListEditor()
{
}

void ListEditor::SetBaseLayout()
{
  QVBoxLayout* pVout = new QVBoxLayout(this);
  QHBoxLayout* pHout = new QHBoxLayout;
  pVout->addWidget(m_List);
  std::array arButtons = {
    new QPushButton(QStringLiteral("添加"), this),
    new QPushButton(QStringLiteral("删除"), this),
    new QPushButton(QStringLiteral("取消"), this),
    new QPushButton(QStringLiteral("确认"), this),
  };

  std::for_each(arButtons.begin(), arButtons.end(),
    std::bind(&QHBoxLayout::addWidget, pHout, std::placeholders::_1, 0, Qt::Alignment()));
  
  for (const auto& item: m_Data.getArray()) {
    std::u8string_view str = item.getValueString();
    m_List->addItem(QString::fromUtf8(str.data(), str.size()));
  }

  pVout->addLayout(pHout);

  connect(arButtons[0], &QPushButton::clicked, this, [this]() {
    auto const item = new QListWidgetItem("New Item");
    auto const currentItems = m_List->selectedItems();
    if (currentItems.empty()) {
      m_List->addItem(item);
    } else {
      m_List->insertItem(m_List->currentRow(), item);
    }
    const QString str = QInputDialog::getText(
        this, m_ArgEditTitle, m_ArgEditLabel, 
        QLineEdit::Normal, item->text());
    if (!str.isEmpty() && str != item->text())
      item->setText(str);
  });

  connect(arButtons[1], &QPushButton::clicked, m_List, [this]() {
    for (auto item: m_List->selectedItems()) {
      delete item;
    }
  });

  connect(arButtons[2], &QPushButton::clicked, this, &ListEditor::close);

  connect(arButtons[3], &QPushButton::clicked, this, &ListEditor::SaveData);

  connect(m_List, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item){
    const QString str = QInputDialog::getText(
        this, m_ArgEditTitle, m_ArgEditLabel,
        QLineEdit::Normal, item->text());
    if (!str.isEmpty() && str != item->text())
      item->setText(str);
  });
}

void ListEditor::SaveData() {
  std::list<YJson> args;
  for (int i = 0; i < m_List->count(); ++i) {
    auto ptr = m_List->item(i);
    if (ptr == nullptr) continue;
    const QString arg = ptr->text();
    if (arg.isEmpty()) continue;
    args.emplace_back(PluginObject::QString2Utf8(arg));
  }
  m_Data.getArray() = std::move(args);
  m_DataChanged = true;
  close();
}
