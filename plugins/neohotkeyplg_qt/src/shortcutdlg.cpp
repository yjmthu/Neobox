#include <shortcutdlg.h>
#include <pluginmgr.h>
#include <pluginobject.h>
#include <hotkeyitemwidget.h>
#include <yjson.h>
#include <neoapp.h>

#include <ui_shortcutdlg.h>

#include <QEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPushButton>


ShortcutDlg::ShortcutDlg(class YJson& setting)
  : QDialog(nullptr)
  , ui(new Ui::Dialog)
  , m_Setting(setting)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  ui->setupUi(this);
  InitLayout();
  InitConnect();
}

ShortcutDlg::~ShortcutDlg()
{
  delete ui;
}

void ShortcutDlg::InitLayout()
{
  
  ui->listWidget->setResizeMode(QListView::Adjust);
  // ui->listWidget->setFlow(QListView::LeftToRight); 
  // ui->listWidget->setViewMode(QListView::IconMode);
  
  for (const auto& info: m_Setting.getArray()) {
    auto item = new QListWidgetItem;
    item->setSizeHint(QSize(400, 40));
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, new HotKeyItemWidget(ui->listWidget, info));
  }
}

void ShortcutDlg::InitConnect()
{
  connect(ui->pBtnClear, &QPushButton::clicked, ui->listWidget, &QListWidget::clear);
  connect(ui->pBtnAdd, &QPushButton::clicked, this, [this](){
    auto item = new QListWidgetItem;
    item->setSizeHint(QSize(400, 40));
    ui->listWidget->insertItem(ui->listWidget->currentRow(), item);
    ui->listWidget->setItemWidget(item, new HotKeyItemWidget(ui->listWidget, YJson(YJson::O {
      {u8"KeySequence", YJson::String},
      {u8"Enabled", false},
      {u8"Plugin", YJson::String}
    })));
  });
  connect(ui->pBtnRemove, &QPushButton::clicked, this, [this](){
    auto const item = ui->listWidget->currentItem();
    // delete ui->listWidget->itemWidget(item);
    delete item;
  });
  connect(ui->pBtnSave, &QPushButton::clicked, this, &ShortcutDlg::SaveSetting);
}

void ShortcutDlg::SaveSetting()
{
  m_Setting.clearA();
  for (int i = 0; i < ui->listWidget->count(); ++i) {
    auto item = qobject_cast<HotKeyItemWidget*>(ui->listWidget->itemWidget(ui->listWidget->item(i)));
    item->WriteJson(m_Setting);
  }
  mgr->SaveSettings();
  emit finished();
  close();
  glb->glbShowMsg("更新热键列表成功！");
}

