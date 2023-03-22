#include <translatedlg.hpp>
#include <translate.h>
#include <pluginmgr.h>
#include <pluginobject.h>
#include <heightctrl.hpp>
#include <yjson.h>
#include <menubase.hpp>
#include <neomenu.hpp>

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextBlock>
#include <QVBoxLayout>
#include <QMimeData>
#include <QWidget>

NeoTranslateDlg::NeoTranslateDlg(YJson& settings)
    : WidgetBase(mgr->m_Menu, true)
    , m_Settings(settings)
    , m_CenterWidget(new QWidget(this))
    , m_TextFrom(new QPlainTextEdit(m_CenterWidget))
    , m_TextTo(new QTextEdit(m_CenterWidget))
    , m_BoxFrom(new QComboBox(m_CenterWidget))
    , m_BoxTo(new QComboBox(m_CenterWidget))
    , m_Translate(new Translate(m_Settings, [this](const void* data, size_t size){
        if (m_Translate->GetSource() == Translate::Baidu)
          m_TextTo->setPlainText(QString::fromUtf8(reinterpret_cast<const char*>(data), size));
        else
          m_TextTo->setHtml(QString::fromUtf8(reinterpret_cast<const char*>(data), size));
      }))
    , m_HeightCtrl(new HeightCtrl(this, settings[u8"HeightRatio"]))
    , m_BtnReverse(new QPushButton(m_CenterWidget))
    , m_BtnCopyFrom(new QPushButton(m_TextFrom))
    , m_BtnCopyTo(new QPushButton(m_TextTo))
    , m_BtnTransMode(new QPushButton("查词", m_CenterWidget))
{
  SetupUi();
  m_BtnCopyFrom->setText("复制");
  m_BtnCopyTo->setText("复制");
  connect(m_BtnCopyFrom, &QPushButton::clicked, this, [this]() {
    QClipboard* clip = QApplication::clipboard();
    clip->setText(m_TextFrom->toPlainText());
    m_BtnCopyFrom->setText("成功");
  });
  connect(m_BtnCopyTo, &QPushButton::clicked, this, [this]() {
    QClipboard* clip = QApplication::clipboard();
    clip->setText(m_TextTo->toPlainText());
    m_BtnCopyTo->setText("成功");
  });
  m_BtnReverse->setObjectName("reverseLanguage");
  m_BtnCopyFrom->setObjectName("copyTextFrom");
  m_BtnCopyTo->setObjectName("copyTextTo");
  m_BtnCopyFrom->setCursor(Qt::PointingHandCursor);
  m_BtnCopyTo->setCursor(Qt::PointingHandCursor);
  SetStyleSheet();

  m_TextFrom->installEventFilter(this);
  m_TextTo->installEventFilter(this);

  AddScrollBar(m_TextFrom->verticalScrollBar());
  AddScrollBar(m_TextTo->verticalScrollBar());

  const auto& position = m_Settings[u8"Position"].getArray();
  auto const& size = m_Settings[u8"Size"].getArray();
  m_LastPostion = QPoint {
    position.front().getValueInt(), position.back().getValueInt()
  };
  m_LastSize = QSize {
    size.front().getValueInt(), size.back().getValueInt()
  };
}

NeoTranslateDlg::~NeoTranslateDlg() {
  delete m_HeightCtrl;
  delete m_Translate;
  delete m_TextFrom;
  delete m_TextTo;
}

void NeoTranslateDlg::GetResultData(QUtf8StringView text) {
  if (!m_TextFromChanged) return;
  m_TextTo->clear();
  if (text.size() != 0) {
     m_Translate->GetResult(text);
  }
  m_TextFromChanged = false;
}

void NeoTranslateDlg::showEvent(QShowEvent* event) {
  static auto const defaultSize = size();

  resize(m_Settings[u8"AutoSize"].isTrue() ? defaultSize : m_LastSize);

  if (m_Settings[u8"AutoMove"].isTrue()) {
    auto const speedbox = ReferenceObject();
    if (speedbox) {
      const auto qFormRect = speedbox->frameGeometry();

      const auto size = QGuiApplication::primaryScreen()->size();
      const auto mSize = frameSize();
      int x, y;
      y = (mSize.height() + qFormRect.bottom() > size.height())
              ? qFormRect.top() - mSize.height()
              : qFormRect.bottom();
      if (((mSize.width() + qFormRect.width()) >> 1) + qFormRect.left() >
          size.width()) {
        x = size.width() - mSize.width();
      } else if ((qFormRect.left() << 1) + qFormRect.width() < mSize.width()) {
        x = 0;
      } else {
        x = qFormRect.right() - ((qFormRect.width() + mSize.width()) >> 1);
      }
      m_LastPostion = QPoint(x, y);
    }
  }

  move(m_LastPostion);

  m_BtnCopyFrom->move(m_TextFrom->width() - m_BtnCopyFrom->width() - 4, 4);
  m_BtnCopyTo->move(m_TextTo->width() - m_BtnCopyTo->width() - 4, 4);
  m_BtnCopyFrom->hide();
  m_BtnCopyTo->hide();

  m_HeightCtrl->UpdateUi();

  m_TextFrom->setFocus();
  activateWindow();

  if (m_Settings[u8"AutoTranslate"].isTrue()) {
    if (!m_TextFrom->toPlainText().isEmpty()) {
      GetResultData(m_TextFrom->toPlainText().toUtf8());
    }
  }
  this->WidgetBase::showEvent(event);
}

void NeoTranslateDlg::hideEvent(QHideEvent *event)
{
  bool save = false;
  if (m_LastPostion != pos()) {
    if (m_Settings[u8"AutoMove"].isFalse()) {
      m_LastPostion = pos();
      m_Settings[u8"Position"] = YJson::A {
        m_LastPostion.x(),
        m_LastPostion.y(),
      };
      save = true;
    }
  }
  if (m_LastSize != size()) {
    if (m_Settings[u8"AutoSize"].isFalse()) {
      m_LastSize = size();
      m_Settings[u8"Size"] = YJson::A {
        m_LastSize.width(), m_LastSize.height()
      };
      save = true;
    }
  }
  if (m_LanPairChanged) {
    m_LanPairChanged = false;
    save = true;
  }
  if (save) mgr->SaveSettings();
  event->accept();
}

bool NeoTranslateDlg::eventFilter(QObject* target, QEvent* event) {
  // static bool bShiftDown = false, bCtrlDown = false;
  if (target == m_TextFrom || target == m_TextTo) {
    switch (event->type()) { 
    case QEvent::KeyPress: 
      switch (auto const keyEvent = reinterpret_cast<QKeyEvent*>(event); keyEvent->key()) {
        case Qt::Key_Escape:
          close();
          return true;
        case Qt::Key_Return: {
          if (keyEvent->modifiers() & Qt::ControlModifier) {
            if (target == m_TextFrom)
              m_TextFrom->insertPlainText("\n");
            else if (target == m_TextTo)
              m_TextTo->insertPlainText("\n");
          } else {
            GetResultData(m_TextFrom->toPlainText().toUtf8());
          }
          return true;
        }
        case Qt::Key_Tab: {
          if (target == m_TextFrom) {
            if (keyEvent->modifiers() & Qt::ControlModifier) {
              m_TextFrom->insertPlainText("\t");
            } else {
              ReverseLanguage();
              return true;
            }
          }
          break;
        }
        case Qt::Key_Space: {
          if (keyEvent->modifiers() & Qt::ShiftModifier) {
            ReverseLanguage();
            return true;
          }
          break;
        }
        case Qt::Key_Up:
          if (keyEvent->modifiers() & Qt::AltModifier) {
            int i = m_BoxTo->currentIndex();
            m_BoxTo->setCurrentIndex(i == 0 ? m_BoxTo->count() - 1 : --i);
            m_Translate->m_LanPair->to = m_BoxTo->currentIndex();
            m_LanPairChanged = true;
            return true;
          }
          break;
        case Qt::Key_Down:
          if (keyEvent->modifiers() & Qt::AltModifier) {
            int i = m_BoxTo->currentIndex();
            m_BoxTo->setCurrentIndex(++i == m_BoxTo->count() ? 0 : i);
            m_Translate->m_LanPair->to = m_BoxTo->currentIndex();
            m_LanPairChanged = true;
            return true;
          }
          break;
        case Qt::Key_Left:
          if (keyEvent->modifiers() & Qt::AltModifier) {
            int i = m_BoxFrom->currentIndex();
            m_BoxFrom->setCurrentIndex(i == 0 ? m_BoxFrom->count() - 1 : --i);
            m_Translate->m_LanPair->to = m_BoxFrom->currentIndex();
            m_LanPairChanged = true;
            return true;
          }
          break;
        case Qt::Key_Right:
          if (keyEvent->modifiers() & Qt::AltModifier) {
            int i = m_BoxFrom->currentIndex();
            m_BoxFrom->setCurrentIndex(++i == m_BoxFrom->count() ? 0 : i);
            m_Translate->m_LanPair->to = m_BoxFrom->currentIndex();
            m_LanPairChanged = true;
            return true;
          }
          break;
        case Qt::Key_M:
          if (keyEvent->modifiers() & Qt::ControlModifier) {
            m_BtnTransMode->toggle();
            return true;
          }
        default:
          break;
      }
      break;
    case QEvent::Enter:
      if (target == m_TextFrom) {
        m_BtnCopyFrom->show();
      } else {
        m_BtnCopyTo->show();
      }
      break;
    case QEvent::Leave:
      if (target == m_TextFrom) {
        m_BtnCopyFrom->hide();
        m_BtnCopyFrom->setText(QStringLiteral("复制"));
      } else {
        m_BtnCopyTo->hide();
        m_BtnCopyTo->setText(QStringLiteral("复制"));
      }
      break;
    case QEvent::MouseButtonPress:
      if (target == m_TextFrom) {
        CreateFromRightMenu(dynamic_cast<QMouseEvent*>(event));
      } else if (target == m_TextTo) {
        CreateToRightMenu(dynamic_cast<QMouseEvent*>(event));
      }
      break;
    default:
      break;
    }
  }
  return WidgetBase::eventFilter(target, event);
}

void NeoTranslateDlg::ToggleVisibility()
{
  if (isVisible()) {
    hide();
  } else {
    const auto set = m_Settings[u8"ReadClipboard"];
    if (set.isTrue()) {
      const auto *clipbord = QGuiApplication::clipboard();
      const auto *mimeData = clipbord->mimeData();
      if (mimeData->hasText()) {
        m_TextFrom->setPlainText(mimeData->text());
        return show();
      }
    }
    show();
  }
}

void NeoTranslateDlg::SetupUi()
{
  setWindowTitle("极简翻译");
  setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
  setAttribute(Qt::WA_TranslucentBackground);
  auto const mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(m_CenterWidget);
  m_CenterWidget->setObjectName("grayBackground");
  AddTitle("极简翻译");
  AddCloseButton();
  SetShadowAround(m_CenterWidget);

  m_TextFrom->setObjectName("neoTextFrom");
  m_TextTo->setObjectName("neoTextTo");
  m_TextTo->setReadOnly(true);
  m_TextFrom->setMinimumHeight(30);
  m_TextFrom->setMaximumHeight(30);
  auto const pvLayout = new QVBoxLayout(m_CenterWidget);
  pvLayout->setContentsMargins(11, 30, 11, 11);
  pvLayout->addWidget(m_TextFrom);
  pvLayout->addWidget(m_TextTo);
  auto const phLayout = new QHBoxLayout;
  AddCombbox(phLayout);
  auto const pButtonGet = new QPushButton("翻译", m_CenterWidget);
  phLayout->addWidget(pButtonGet);
  pvLayout->addLayout(phLayout);
  connect(pButtonGet, &QPushButton::clicked, this, [this](){
    GetResultData(m_TextFrom->toPlainText().toUtf8());
  });
}

void NeoTranslateDlg::SetStyleSheet()
{
  setStyleSheet(
    "QPushButton#copyTextTo,"
    "QPushButton#copyTextFrom {"
      "background-color: #9999AA;"
      "color: #ffffff;"
      "border-radius: 4px;"
      "padding: 2px 8px;"
      "font-size: 12px;"
    "}"

    "QPushButton#btnTransMode {"
      "background-color: white;"
    "}"

    "QPushButton#btnTransMode:checked {"
      "background-color: blueviolet;"
    "}"

    "QPushButton#btnTransMode:hover {"
      "background-color: slategray;"
    "}"

    "QPushButton#reverseLanguage {"
      "background-color: rgba(200, 200, 200, 100);"
      "border-image: url(:/icons/reversible-arraow-forward.png)"
    "}"

    "QPushButton#reverseLanguage:checked {"
      "background-color: rgba(200, 200, 200, 100);"
      "border-image: url(:/icons/reversible-arraow-backward.png)"
    "}"

    "QPushButton#reverseLanguage:hover {"
      "background-color: rgba(150, 150, 150, 150);"
    "}"

    "QPushButton#reverseLanguage:pressed {"
      "background-color: rgba(100, 100, 100, 200);"
    "}"
  );
}

QWidget* NeoTranslateDlg::ReferenceObject() const
{
  return qobject_cast<QWidget*>(PluginObject::GetMainObject(u8"neospeedboxplg"));
}

void NeoTranslateDlg::ReverseLanguage()
{
  auto const pair = m_Translate->ReverseLanguage();
  if (pair) {
    m_BoxFrom->setCurrentIndex(pair->first);
    m_BoxTo->setCurrentIndex(pair->second);
  } else {
    mgr->ShowMsg("无法转化语言！");
  }
}

void NeoTranslateDlg::ChangeLanguageSource(bool checked) {
  auto const newDict = checked ? Translate::Youdao: Translate::Baidu;
  m_TextFromChanged = true;

  m_BoxFrom->disconnect(this);
  m_BoxFrom->clear();
  m_BoxTo->disconnect(this);
  m_BoxTo->clear();

  m_Translate->SetSource(newDict);

  for (const auto& [from, _]: m_Translate->m_LanguageCanFromTo[newDict]) {
    const auto& name = m_Translate->m_LangNameMap[from];
    m_BoxFrom->addItem(PluginObject::Utf82QString(name));
  }
  const auto& [_, tos] = m_Translate->m_LanguageCanFromTo[newDict][m_Translate->m_LanPair->f()];
  for (auto& to : tos) {
    const auto& name = m_Translate->m_LangNameMap[to];
    m_BoxTo->addItem(PluginObject::Utf82QString(name));
  }

  m_BoxFrom->setCurrentIndex(m_Translate->m_LanPair->f());
  m_BoxTo->setCurrentIndex(m_Translate->m_LanPair->t());

  connect(m_BoxFrom, &QComboBox::currentIndexChanged, this, &NeoTranslateDlg::ChangeLanguageFrom);
  connect(m_BoxTo, &QComboBox::currentIndexChanged, this, &NeoTranslateDlg::ChangeLanguageTo);

  m_Settings[u8"Mode"] = checked ? 1 : 0;

  m_HeightCtrl->UpdateUi();
  m_LanPairChanged = true;
}

void NeoTranslateDlg::ChangeLanguageFrom(int index) {
  m_Translate->m_LanPair->from = index;
  m_Translate->m_LanPair->to = 0;
  m_LanPairChanged = true;
  m_TextFromChanged = true;

  m_BoxTo->disconnect(this);
  m_BoxTo->clear();

  for (auto& to: m_Translate->m_LanguageCanFromTo[m_Translate->GetSource()][index].second) {
      const auto& name = m_Translate->m_LangNameMap[to];
      m_BoxTo->addItem(PluginObject::Utf82QString(name));
  }

  connect(m_BoxTo, &QComboBox::currentIndexChanged, this, &NeoTranslateDlg::ChangeLanguageTo);
}

void NeoTranslateDlg::ChangeLanguageTo(int index)
{
  m_Translate->m_LanPair->to = index;
  m_LanPairChanged = true;
  m_TextFromChanged = true;
}

void NeoTranslateDlg::CreateToRightMenu(QMouseEvent* event)
{
  if (!(event->buttons() & Qt::MouseButton::RightButton)) return;
    // m_TextTo->setContextMenuPolicy(Qt::CustomContextMenu);
  auto const toMenu = m_TextTo->createStandardContextMenu();
  auto const clearAction = toMenu->addAction("Clear");
  QObject::connect(clearAction, &QAction::triggered, m_TextTo, &QTextEdit::clear);
  auto const searchAction = toMenu->addAction("Search");
  QObject::connect(searchAction, &QAction::triggered, m_TextTo, [this](){
    auto const text = m_TextTo->textCursor().selectedText();
    if (text.isEmpty()) {
      GetResultData(m_TextFrom->toPlainText().toUtf8());
    } else {
      m_TextFrom->setPlainText(text);
      GetResultData(text.toUtf8());
    }
  });
  toMenu->exec(event->globalPosition().toPoint());
  delete toMenu;
}

void NeoTranslateDlg::CreateFromRightMenu(QMouseEvent* event)
{
  if (!(event->buttons() & Qt::MouseButton::RightButton)) return;
  // m_TextFrom->setContextMenuPolicy(Qt::CustomContextMenu);
  auto const fromMenu = m_TextFrom->createStandardContextMenu();
  auto const clearAction = fromMenu->addAction("Clear");
  QObject::connect(clearAction, &QAction::triggered, m_TextFrom, [this](){
    auto cursor = m_TextFrom->textCursor();
    if (cursor.selectedText().isEmpty()) {
      m_TextFrom->clear();
    } else {
      cursor.removeSelectedText();
      m_TextFrom->setTextCursor(cursor);
    }
  });
  auto const searchAction = fromMenu->addAction("Search");
  QObject::connect(searchAction, &QAction::triggered, m_TextFrom, [this](){
    auto const text = m_TextFrom->textCursor().selectedText();
    if (text.isEmpty()) {
      GetResultData(m_TextFrom->toPlainText().toUtf8());
    } else {
      m_TextFrom->setPlainText(text);
      GetResultData(text.toUtf8());
    }
  });
  fromMenu->exec(event->globalPosition().toPoint());
  delete fromMenu;
}

void NeoTranslateDlg::AddCombbox(QHBoxLayout* layout) {
  layout->addWidget(m_BoxFrom);
  layout->addWidget(m_BtnReverse);
  layout->addWidget(m_BoxTo);
  layout->addWidget(m_HeightCtrl);

  m_BtnReverse->setCheckable(true);
  m_BtnTransMode->setObjectName(QStringLiteral("btnTransMode"));
  m_BtnTransMode->setCheckable(true);
  layout->addWidget(m_BtnTransMode);
  auto const dict = static_cast<Translate::Source>(m_Settings[u8"Mode"].getValueInt());
  m_Translate->SetSource(dict);
  m_BtnTransMode->setChecked(dict == Translate::Youdao);
  ChangeLanguageSource(dict);
  m_LanPairChanged = false;

  connect(m_BtnReverse, &QPushButton::clicked, this, &NeoTranslateDlg::ReverseLanguage);
  connect(m_TextFrom, &QPlainTextEdit::textChanged, this, [this](){m_TextFromChanged = true;});
  connect(m_BtnTransMode, &QPushButton::toggled, this, &NeoTranslateDlg::ChangeLanguageSource);
  connect( m_BoxFrom, &QComboBox::currentIndexChanged, this, &NeoTranslateDlg::ChangeLanguageFrom);
  connect(m_BoxTo, &QComboBox::currentIndexChanged, this, &NeoTranslateDlg::ChangeLanguageTo);

}
