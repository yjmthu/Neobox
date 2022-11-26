#include <translatedlg.h>
#include <translate.h>
#include <pluginobject.h>
#include <yjson.h>

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextBlock>
#include <QVBoxLayout>
#include <QMimeData>
#include <QWidget>

NeoTranslateDlg::NeoTranslateDlg(YJson& settings)
    : QWidget(nullptr, Qt::WindowStaysOnTopHint | Qt::Tool),
      m_Settings(settings),
      m_TextFrom(new QPlainTextEdit(this)),
      m_TextTo(new QPlainTextEdit(this)),
      m_BoxFrom(new QComboBox(this)),
      m_BoxTo(new QComboBox(this)),
      m_Translate(new Translate),
      m_BtnCopyFrom(new QPushButton(m_TextFrom)),
      m_BtnCopyTo(new QPushButton(m_TextTo)),
      m_BtnTransMode(new QPushButton(this)) {
  setWindowTitle(QStringLiteral("极简翻译"));
  QVBoxLayout* pvLayout = new QVBoxLayout(this);
  pvLayout->addWidget(m_TextFrom);
  pvLayout->addWidget(m_TextTo);
  QHBoxLayout* phLayout = new QHBoxLayout;
  AddCombbox(phLayout);
  QPushButton* pButtonGet = new QPushButton(QStringLiteral("翻译"), this);
  phLayout->addWidget(pButtonGet);
  pvLayout->addLayout(phLayout);
  connect(pButtonGet, &QPushButton::clicked, this,
          std::bind(&NeoTranslateDlg::GetResultData, this));

  m_BtnCopyFrom->setText(QStringLiteral("复制"));
  m_BtnCopyTo->setText(QStringLiteral("复制"));
  connect(m_BtnCopyFrom, &QPushButton::clicked, this, [this]() {
    QClipboard* clip = QApplication::clipboard();
    clip->setText(m_TextFrom->toPlainText());
    m_BtnCopyFrom->setText(QStringLiteral("成功"));
  });
  connect(m_BtnCopyTo, &QPushButton::clicked, this, [this]() {
    QClipboard* clip = QApplication::clipboard();
    clip->setText(m_TextTo->toPlainText());
    m_BtnCopyTo->setText(QStringLiteral("成功"));
  });
  m_BtnCopyFrom->setObjectName("copyTextFrom");
  m_BtnCopyTo->setObjectName("copyTextTo");
  m_BtnCopyFrom->setCursor(Qt::PointingHandCursor);
  m_BtnCopyTo->setCursor(Qt::PointingHandCursor);

  m_TextFrom->installEventFilter(this);
  m_TextTo->installEventFilter(this);
}

NeoTranslateDlg::~NeoTranslateDlg() {
  delete m_Translate;
  delete m_TextFrom;
  delete m_TextTo;
}

void NeoTranslateDlg::showEvent(QShowEvent*) {
  QSize size = QGuiApplication::primaryScreen()->size();
  QSize mSize = frameSize();
  int x, y;
  y = (mSize.height() + m_FormRect.bottom() > size.height())
          ? m_FormRect.top() - mSize.height()
          : m_FormRect.bottom();
  if (((mSize.width() + m_FormRect.width()) >> 1) + m_FormRect.left() >
      size.width()) {
    x = size.width() - mSize.width();
  } else if ((m_FormRect.left() << 1) + m_FormRect.width() < mSize.width()) {
    x = 0;
  } else {
    x = m_FormRect.right() - ((m_FormRect.width() + mSize.width()) >> 1);
  }
  move(x, y);

  m_BtnCopyFrom->move(m_TextFrom->width() - m_BtnCopyFrom->width() - 4, 4);
  m_BtnCopyTo->move(m_TextTo->width() - m_BtnCopyTo->width() - 4, 4);
  m_BtnCopyFrom->hide();
  m_BtnCopyTo->hide();

  m_TextFrom->setFocus();
}

bool NeoTranslateDlg::eventFilter(QObject* target, QEvent* event) {
  static bool bShiftDown = false, bCtrlDown = false;
  if (target == m_TextFrom || target == m_TextTo) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* keyEvent = reinterpret_cast<QKeyEvent*>(event);
      switch (keyEvent->key()) {
        case Qt::Key_Escape:
          close();
          return true;
        case Qt::Key_Return: {
          if (bCtrlDown || m_Translate->GetDict() == Translate::Dict::Youdao) {
            GetResultData();
            return true;
          }
          break;
        }
        case Qt::Key_Alt:
          if (int i = m_BoxTo->currentIndex(); bShiftDown) {
            m_BoxTo->setCurrentIndex(i == 0 ? m_BoxTo->count() - 1 : --i);
          } else {
            m_BoxTo->setCurrentIndex(++i == m_BoxTo->count() ? 0 : i);
          }
          return true;
        case Qt::Key_Shift:
          bShiftDown = true;
          return true;
        case Qt::Key_Control:
          bCtrlDown = true;
          return true;
        case Qt::Key_M:
          if (bCtrlDown) {
            m_BtnTransMode->setChecked(!m_BtnTransMode->isChecked());
            return true;
          }
        default:
          break;
      }
    } else if (event->type() == QEvent::KeyRelease) {
      QKeyEvent* keyEvent = reinterpret_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_Shift) {
        bShiftDown = false;
        return true;
      } else if (keyEvent->key() == Qt::Key_Control) {
        bCtrlDown = false;
        return true;
      }
    } else if (event->type() == QEvent::Enter) {
      if (target == m_TextFrom) {
        m_BtnCopyFrom->show();
      } else {
        m_BtnCopyTo->show();
      }
    } else if (event->type() == QEvent::Leave) {
      if (target == m_TextFrom) {
        m_BtnCopyFrom->hide();
        m_BtnCopyFrom->setText(QStringLiteral("复制"));
      } else {
        m_BtnCopyTo->hide();
        m_BtnCopyTo->setText(QStringLiteral("复制"));
      }
    }
  }
  return QWidget::eventFilter(target, event);
}

void NeoTranslateDlg::ToggleVisibility()
{
   if (isVisible()) {
     hide();
   } else {
     const auto set = m_Settings[u8"ReadClipboard"];
     if (set.isTrue()) {
       const QClipboard *clipbord = QGuiApplication::clipboard();
       const QMimeData *mimeData = clipbord->mimeData();
       if (mimeData->hasText()) {
         Show(mimeData->text());
         return;
       }
     }
     Show();
   }
}

QWidget* NeoTranslateDlg::ReferenceObject() const
{
  return qobject_cast<QWidget*>(PluginObject::GetMainObject(u8"neospeedboxplg"));
}

void NeoTranslateDlg::Show(const QString& text) {
  if (!ReferenceObject()) return;
  m_FormRect = ReferenceObject()->frameGeometry();
  m_TextFrom->setPlainText(text);
  m_TextTo->clear();
  show();
  activateWindow();

  if (m_Settings[u8"AutoTranslate"].isTrue()) {
    if (!text.isEmpty())
      GetResultData();
  }
}

void NeoTranslateDlg::Show() {
  if (!ReferenceObject()) return;
  m_FormRect = ReferenceObject()->frameGeometry();
  show();
  activateWindow();
}
void NeoTranslateDlg::GetResultData() {
  m_Translate->m_LanPair = {m_BoxFrom->currentIndex(), m_BoxTo->currentIndex()};
  QByteArray array = m_TextFrom->toPlainText().toUtf8();
  auto result =
      m_Translate->GetResult(std::u8string(array.begin(), array.end()));
  m_TextTo->clear();
  if (m_Translate->IsUsingBaidu())
    m_TextTo->appendPlainText(QString::fromUtf8(result.data(), result.size()));
  else
    m_TextTo->appendHtml(QString::fromUtf8(result.data(), result.size()));
}

void NeoTranslateDlg::ChangeLanguage(int from) {
  static auto lastDict = (Translate::Dict)-1;
  auto curDict = m_Translate->GetDict();
  m_BoxTo->clear();

  disconnect(m_BoxFrom, &QComboBox::currentIndexChanged, 0, 0);

  if (curDict == Translate::Dict::Baidu) {
    auto& vec = m_Translate->m_LanNamesBaidu[from].second;
    if (curDict != lastDict) {
      if (m_BoxFrom->count())
        m_BoxFrom->clear();
      for (auto& [i, j] : m_Translate->m_LanNamesBaidu) {
        const auto& name = m_Translate->m_LanMap[i];
        const QString&& qsName = QString::fromUtf8(name.data(), name.size());
        m_BoxFrom->addItem(qsName);
      }
    }
    for (auto& i : vec) {
      const auto& name = m_Translate->m_LanMap[i];
      m_BoxTo->addItem(QString::fromUtf8(name.data(), name.size()));
    }
  } else if (curDict == Translate::Dict::Youdao) {
    auto& vec = m_Translate->m_LanNamesYoudao[from].second;
    if (curDict != lastDict) {
      if (m_BoxFrom->count())
        m_BoxFrom->clear();
      for (auto& [i, j] : m_Translate->m_LanNamesYoudao) {
        const auto& name = m_Translate->m_LanMap[i];
        const QString&& qsName = QString::fromUtf8(name.data(), name.size());
        m_BoxFrom->addItem(qsName);
      }
    }
    for (auto& i : vec) {
      const auto& name = m_Translate->m_LanMap[i];
      m_BoxTo->addItem(QString::fromUtf8(name.data(), name.size()));
    }
  }

  connect(
      m_BoxFrom, &QComboBox::currentIndexChanged, this,
      std::bind(&NeoTranslateDlg::ChangeLanguage, this, std::placeholders::_1));

  m_Translate->m_LanPair.second = 0;
  lastDict = curDict;
}

void NeoTranslateDlg::AddCombbox(QHBoxLayout* layout) {
  layout->addWidget(m_BoxFrom);
  QLabel* lable = new QLabel(this);
  lable->setText(QStringLiteral("→"));
  layout->addWidget(lable);
  layout->addWidget(m_BoxTo);

  m_BtnTransMode->setObjectName(QStringLiteral("btnTransMode"));
  m_BtnTransMode->setCheckable(true);
  m_BtnTransMode->setText(QStringLiteral("查词"));
  layout->addWidget(m_BtnTransMode);
  auto dic = static_cast<Translate::Dict>(m_Settings[u8"Mode"].getValueInt());
  m_Translate->SetDict(dic);
  m_BtnTransMode->setChecked(dic == Translate::Dict::Youdao);

  connect(m_BtnTransMode, &QPushButton::toggled, this, [this](bool checked) {
    auto dic = Translate::Dict::Baidu;
    if (checked)
      dic = Translate::Dict::Youdao;
    m_Settings[u8"Mode"] = static_cast<int>(dic);
    m_Translate->SetDict(dic);
    PluginObject::SaveSettings();
    ChangeLanguage();
  });

  connect(m_BoxTo, &QComboBox::currentIndexChanged, this,
          [this](int index) { m_Translate->m_LanPair.second = index; });
  ChangeLanguage();
}
