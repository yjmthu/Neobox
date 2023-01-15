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

#include <windows.h>

ColorDlg* ColorDlg::m_Instance = nullptr;
HHOOK ColorDlg::m_Hoock[2] { nullptr, nullptr };

void ColorDlg::SaveTopState(bool isTop)
{
  m_Settings[u8"StayTop"] = isTop;
  mgr->SaveSettings();
}

ColorDlg::ColorDlg(YJson& settings, QWidget* parent)
  : WidgetBase(parent, false, settings[u8"StayTop"].isTrue())
  , m_Settings(settings)
  , m_ColorsArray(settings[u8"History"])
  , m_CenterWidget(new QWidget(this))
  , ui(new Ui::ColorForm)
  , m_SmallForm(nullptr)
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
  if (m_SmallForm) {
    m_SmallForm->close();
    m_SmallForm = nullptr;
  }
  SaveHistory();
  if (!UninstallHook()) {
    mgr->ShowMsg(QStringLiteral("卸载钩子失败！\n错误代码【%1】").arg(GetLastError()));
  }
  m_Instance = nullptr;
}

void ColorDlg::PickColor()
{
  if (m_SmallForm) {
    mgr->ShowMsg("正在拾取颜色！");
    return;
  }
  if (!InstallHook()) {
    mgr->ShowMsg(QStringLiteral("注册钩子失败！\n错误代码【%1】。").arg(GetLastError()));
    return;
  }
  POINT pt;
  if (!GetCursorPos(&pt)) {
    mgr->ShowMsg(QStringLiteral("获取鼠标位置失败！\n错误代码【%1】。").arg(GetLastError()));
    return;
  }
  m_SmallForm = new SmallForm;
  m_SmallForm->m_Position = QPoint(pt.x, pt.y);
  m_SmallForm->show();

  hide();
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
  {
    QFile file(":/styles/neocolorplg-scrollbar-wide.qss");
    file.open(QFile::ReadOnly);
    m_StyleWide = file.readAll();
    file.close();
  }
  {
    QFile file(":/styles/neocolorplg-scrollbar-narrow.qss");
    file.open(QFile::ReadOnly);
    m_StyleNarrow = file.readAll();
    file.close();
  }
  ui->listWidget->verticalScrollBar()->setStyleSheet(m_StyleNarrow);
  ui->listWidget->verticalScrollBar()->installEventFilter(this);
}

void ColorDlg::InitSignals()
{
  auto const fnCopy = [](QLabel* label) {
    QApplication::clipboard()->setText(label->text());
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

  connect(ui->btnPick, &QPushButton::clicked, this, &ColorDlg::PickColor);

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
  int const max = static_cast<int>(m_Settings[u8"HistoryMaxCount"].getValueDouble());

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

  mgr->SaveSettings();
}

bool ColorDlg::eventFilter(QObject *watched, QEvent *event)
{
  auto const bar = ui->listWidget->verticalScrollBar();
	if (watched == ui->listWidget->verticalScrollBar())
	{
		if (event->type() == QEvent::HoverEnter)
		{
      bar->setFixedWidth(16); //重新定义宽度
      bar->setProperty("STYLE_KEY", QString("SETTINGSSWBG_SCROLL_HOVER")); //重载样式
      bar->setStyleSheet(m_StyleWide);
      // bar->style()->polish(bar); //强制刷新样式
		}
		else if (event->type() == QEvent::HoverLeave)
		{
			bar->setFixedWidth(4);
			bar->setProperty("STYLE_KEY", QString("SETTINGSSWBG_SCROLL"));
      bar->setStyleSheet(m_StyleNarrow);
			// bar->style()->polish(bar);
		}
	}
  return QWidget::eventFilter(watched, event);
}

static LRESULT CALLBACK LowLevelProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  switch (wParam) {
  case WM_LBUTTONDOWN:
    ColorDlg::DoMouseMove(lParam);
    ColorDlg::m_Instance->QuitHook(true);
    return 1;
  case WM_MOUSEWHEEL:
    ColorDlg::DoMouseWheel(lParam);
    return 1;
  case WM_MOUSEMOVE:
    ColorDlg::DoMouseMove(lParam);
    break;
		// PostMessage(
    //   reinterpret_cast<HWND>(ColorDlg::m_Instance->winId()),
    //   wParam, MK_CONTROL, MAKELPARAM(pt.x, pt.y)
    // );
    // break;
  case WM_KEYDOWN: {
		auto const keyHookStruct = (KBDLLHOOKSTRUCT*)lParam;
    if (keyHookStruct->vkCode != VK_ESCAPE) break;
    ColorDlg::m_Instance->QuitHook(false);
		// PostMessage(
    //   reinterpret_cast<HWND>(ColorDlg::m_Instance->winId()),
    //   WM_KEYDOWN, VK_ESCAPE, keyHookStruct->scanCode
    // );
    return 1;
	}
  default:
    break;
  }
	return CallNextHookEx(ColorDlg::m_Hoock[0], nCode, wParam, lParam);
}

bool ColorDlg::InstallHook()
{
  // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowshookexw

  if (!m_Hoock[0]) {
    m_Hoock[0] = SetWindowsHookExW(WH_MOUSE_LL, LowLevelProc, GetModuleHandleW(NULL), 0);
  }
  if (!m_Hoock[1]) {
    m_Hoock[1] = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelProc, GetModuleHandleW(NULL), 0);
  }
  return m_Hoock[0] && m_Hoock[1];
}

bool ColorDlg::UninstallHook()
{
  if (!m_Hoock[0] && !m_Hoock[1]) {
    return true;
  }
  if ((!m_Hoock[0] || UnhookWindowsHookEx(m_Hoock[0]))
      &&
      (!m_Hoock[1] || UnhookWindowsHookEx(m_Hoock[1])))
  {
    m_Hoock[0] = m_Hoock[1] = nullptr;
    return true;
  }
  return false;
}

void ColorDlg::DoMouseMove(LPARAM lParam)
{
  if (!m_Instance || !m_Instance->m_SmallForm) {
    mgr->ShowMsg("失败，未能获取颜色悬浮窗！");
    return;
  }
  auto const mouseHookStruct = (MSLLHOOKSTRUCT*)lParam;
  POINT pt = mouseHookStruct->pt;
  m_Instance->m_SmallForm->m_Position = QPoint(pt.x, pt.y);
  m_Instance->m_SmallForm->GetScreenColor(pt.x, pt.y);
  m_Instance->m_SmallForm->AutoPosition();
}

void ColorDlg::DoMouseWheel(LPARAM lParam)
{
  if (!m_Instance || !m_Instance->m_SmallForm) {
    mgr->ShowMsg("失败，未能获取颜色悬浮窗！");
    return;
  }
  auto const mouseHookStruct = (MSLLHOOKSTRUCT*)lParam;
  POINT pt = mouseHookStruct->pt;
  auto zDelta = GET_WHEEL_DELTA_WPARAM(mouseHookStruct->mouseData);
  m_Instance->m_SmallForm->m_Position = QPoint(pt.x, pt.y);
  m_Instance->m_SmallForm->GetScreenColor(pt.x, pt.y);
  m_Instance->m_SmallForm->AutoPosition();
  m_Instance->m_SmallForm->MouseWheel(zDelta / WHEEL_DELTA);
}

void ColorDlg::QuitHook(bool succeed)
{
  if (m_SmallForm) {
    if (succeed) {
      AddColor(m_SmallForm->m_Color);
    }
    m_SmallForm->close();
    m_SmallForm = nullptr;
  }
  UninstallHook();
  if (succeed) {
    show();
  } else {
    delete this;
  }
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

  auto const item = new QListWidgetItem;
  item->setSizeHint(QSize(66, 46));
  ui->listWidget->insertItem(0, item);
  auto w = new ItemColor(color, ui->listWidget);
  ui->listWidget->setItemWidget(item, w);
  ui->listWidget->setCurrentItem(item);

  SetCurColor(color);
}

/*
bool ColorDlg::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
  if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
  {
    MSG* pMsg = reinterpret_cast<MSG*>(message);
    if (pMsg->message == WM_RBUTTONDOWN)
    {
      POINT pt = pMsg->pt;
      qDebug() << "x="<<pt.x << "y=" << pt.y;
    }
    switch (pMsg->message) {
    case WM_LBUTTONDOWN: {
      break;
    }
    case WM_MOUSEHWHEEL: {
      break;
    }
    case WM_MOUSEMOVE: {
      
      break;
    }
    case WM_KEYDOWN: {
      break;
    }
    default:
      break;
    }
  }
  return WidgetBase::nativeEvent(eventType, message, result);
}
*/
