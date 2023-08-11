#include <colordlg.hpp>
#include <ui_colordlg.h>
#include <yjson.h>
#include <pluginmgr.h>
#include <smallform.hpp>
#include <itemcolor.hpp>
#include <pluginobject.h>
#include <menubase.hpp>

#include <QClipboard>
#include <QVBoxLayout>
#include <QFile>
#include <QScrollBar>

#ifdef _WIN32
#include <windows.h>
#endif

ColorDlg* ColorDlg::m_Instance = nullptr;

void ColorDlg::SaveTopState(bool isTop)
{
  m_Settings.SetStayTop(isTop);
}

ColorDlg::ColorDlg(ColorConfig& settings)
  : WidgetBase(nullptr, false, settings.GetStayTop())
  , m_Settings(settings)
  , m_ColorsArray(m_Settings.GetHistory())
  , m_CenterWidget(new QWidget(this))
  , ui(new Ui::ColorForm)
  , m_ColorsChanged(false)
{
  delete m_Instance;
  m_Instance = this;
  SetupUi();
  SetStyleSheet();
  LoadHistory();
  InitSignals();
}

ColorDlg::~ColorDlg()
{
  delete ui;
  SaveHistory();
  m_Instance = nullptr;
}

void ColorDlg::SetStyleSheet(QWidget* target, const QColor& color)
{
  auto const& daker = color.darker(125);
  const std::string style = std::format(
    "QPushButton {{"
      "background-color: rgb({}, {}, {});"
    "}}"
    "QPushButton:hover {{"
      "background-color: rgb({}, {}, {});"
    "}}", color.red(), color.green(), color.blue(),
    daker.red(), daker.green(), daker.blue());
  target->setStyleSheet(QString::fromStdString(style));
}

void ColorDlg::SetCurItem(int index)
{
  if (index == -1) return;

  auto const item = ui->listWidget->item(index);
  auto w = qobject_cast<ItemColor*>(ui->listWidget->itemWidget(item));
  ui->listWidget->setCurrentItem(item);
  SetCurColor(w->m_Color);
}

void ColorDlg::SetCurColor(const QColor& color)
{
  SetStyleSheet(ui->widget1, color.lighter(200));
  SetStyleSheet(ui->widget2, color.lighter(150));
  SetStyleSheet(ui->widgetCenter, color);
  SetStyleSheet(ui->widget3, color.darker(150));
  SetStyleSheet(ui->widget4, color.darker(200));

  int buffer[6], *rgb = buffer, *hsl = buffer+3;
  color.getRgb(rgb, rgb+1, rgb+2);
  std::string text = std::format("{:02x}{:02x}{:02x}", rgb[0], rgb[1], rgb[2]);
  ui->labelHex->setText(QString::fromStdString(text));

  text = std::format("rgb({}, {}, {})", rgb[0], rgb[1], rgb[2]);
  ui->labelRgb->setText(QString::fromStdString(text));

  color.toHsl().getHsl(hsl, hsl+1, hsl+2);
  text = std::format("hsl({}, {}%, {}%)", hsl[0], hsl[1] * 100 / 255, hsl[2] * 100 / 255);
  ui->labelHsl->setText(QString::fromStdString(text));

  // color.getHsl(hsl, hsl+1, hsl+2);
  // text = std::format("hsl({}, {}, {}", hsl[0], hsl[1], hsl[2]);
  // ui->labelHsl->setText(QString::fromStdString(text));
}

void ColorDlg::RemoveColor(const QColor& color)
{
  auto const name = color.name();
  auto iter = m_Colors.find(name);
  if (iter != m_Colors.end()) {
    m_ColorsArray.removeByValA(PluginObject::QString2Utf8(name));
    m_Settings.SetHistory(m_ColorsArray.getArray());
    m_Colors.erase(iter);
  }
}

void ColorDlg::SetupUi()
{
  setWindowTitle("Neobox-颜色编辑");
  setWindowFlag(Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_DeleteOnClose);

  auto mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(m_CenterWidget);

  ui->setupUi(m_CenterWidget);
  SetShadowAround(m_CenterWidget);
  ui->labelHex->setTextInteractionFlags(Qt::TextSelectableByMouse);
  ui->labelRgb->setTextInteractionFlags(Qt::TextSelectableByMouse);
  ui->labelHsl->setTextInteractionFlags(Qt::TextSelectableByMouse);
  auto const col = QColor(20, 20, 20, 200);
  SetShadowAround(ui->frameHex, 6, col);
  SetShadowAround(ui->frameRgb, 6, col);
  SetShadowAround(ui->frameHsl, 6, col);
  
  AddCloseButton();
  AddMinButton();
  AddTopButton();
}

void ColorDlg::SetStyleSheet()
{
  AddScrollBar(ui->listWidget->verticalScrollBar());
}

void ColorDlg::InitSignals()
{
  auto const fnCopy = [](QLabel* label) {
    QGuiApplication::clipboard()->setText(label->text());
    mgr->ShowMsg("复制成功");
  };
  auto const fnSetColor = [](ColorDlg* that, int ratio, QColor(QColor::* const callback)(int) const) {
    auto const item = that->ui->listWidget->currentItem();
    if (!item) mgr->ShowMsg("不可能出错了！");
    auto const w = qobject_cast<ItemColor*>(that->ui->listWidget->itemWidget(item));
    that->AddColor((w->m_Color.*callback)(ratio));
  };

  connect(ui->btnCopyHex, &QPushButton::clicked, this, std::bind(fnCopy, ui->labelHex));
  connect(ui->btnCopyRgb, &QPushButton::clicked, this, std::bind(fnCopy, ui->labelRgb));
  connect(ui->btnCopyHsl, &QPushButton::clicked, this, std::bind(fnCopy, ui->labelHsl));

  connect(ui->widget1, &QPushButton::clicked, this, std::bind(fnSetColor, this, 200, &QColor::lighter));
  connect(ui->widget2, &QPushButton::clicked, this, std::bind(fnSetColor, this, 150, &QColor::lighter));
  connect(ui->widget3, &QPushButton::clicked, this, std::bind(fnSetColor, this, 150, &QColor::darker));
  connect(ui->widget4, &QPushButton::clicked, this, std::bind(fnSetColor, this, 200, &QColor::darker));

  connect(ui->btnPick, &QPushButton::clicked, this, std::bind(&SmallForm::PickColor, std::ref(m_Settings)));

  connect(ui->listWidget, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* item){
    if (!item) return;
    auto const w = qobject_cast<ItemColor*>(ui->listWidget->itemWidget(item));
    SetCurColor(w->m_Color);
  });

  connect(ui->listWidget, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos){
    auto const item = ui->listWidget->itemAt(pos);
    if (item == nullptr || ui->listWidget->count() < 2) {
      return ;
    }

    auto const popMenu = new MenuBase(ui->listWidget);
    connect(popMenu->addAction("删除"), &QAction::triggered, this, [item, this](){
      auto const color = qobject_cast<ItemColor*>(ui->listWidget->itemWidget(item))->m_Color;
      auto const index = ui->listWidget->row(item);
      delete ui->listWidget->takeItem(index);
      RemoveColor(color);
    });
    popMenu->exec( QCursor::pos() );
    delete popMenu;
  });

}

void ColorDlg::LoadHistory()
{
  for (auto& col: m_ColorsArray.getArray()) {
    const auto& vi = col.getValueString();
    auto const name = QString::fromUtf8(vi.data(), vi.size());
    m_Colors.insert(name);
    auto item = new QListWidgetItem;
    item->setSizeHint(QSize(66, 46));
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, new ItemColor(name, ui->listWidget));
  }

  if (m_ColorsArray.emptyA()) {
    AddColor(QColor(181, 181, 181));
  } else {
    auto const item = ui->listWidget->item(0);
    auto const w = qobject_cast<ItemColor*>(ui->listWidget->itemWidget(item));
    ui->listWidget->setCurrentItem(item);
    SetCurColor(w->m_Color);
  }
}

void ColorDlg::SaveHistory()
{
  if (!m_ColorsChanged) return;

  auto& array = m_ColorsArray.getArray();
  auto iter = array.cbegin();
  int const max = m_Settings.GetHistoryMaxCount();

  for (int i=0; i!=max; ++i) {
    if (iter != array.cend()) {
      ++iter;
    } else {
      break;
    }
  }
  if (iter != array.cend()) {
    array.erase(iter, array.end());
  }

  m_Settings.SetHistory(array);
}

void ColorDlg::AddColor(const QColor& color)
{
  m_ColorsChanged = true;
  auto const name = color.name(QColor::HexRgb);
  auto iter = m_Colors.find(name);
  const QByteArray utf8 = name.toUtf8();
  std::u8string_view str(reinterpret_cast<const char8_t*>(
    utf8.data()), utf8.size());
  if (iter != m_Colors.end()) {
    for (int i=0; i!=ui->listWidget->count(); ++i) {
      auto const item = ui->listWidget->item(i);
      auto const w = qobject_cast<ItemColor*>(ui->listWidget->itemWidget(item));
      if (w->m_Color == color) {
        delete ui->listWidget->takeItem(i);
      }
    }
    m_ColorsArray.removeByValA(str);
  } else {
    m_Colors.insert(name);
  }
  auto& array = m_ColorsArray.getArray();
  array.insert(array.begin(), str);
  m_Settings.SetHistory(array);

  auto const item = new QListWidgetItem;
  item->setSizeHint(QSize(66, 46));
  ui->listWidget->insertItem(0, item);
  auto w = new ItemColor(color, ui->listWidget);
  ui->listWidget->setItemWidget(item, w);
  ui->listWidget->setCurrentItem(item);

  SetCurColor(color);
}
