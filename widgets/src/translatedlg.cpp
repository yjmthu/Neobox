#include <translatedlg.h>
#include <translate.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QClipboard>
#include <QTextBlock>

TranslateDlg::TranslateDlg(QWidget* parent)
    : QWidget(parent, Qt::WindowStaysOnTopHint | Qt::Tool),
      m_TextFrom(new QPlainTextEdit),
      m_TextTo(new QPlainTextEdit),
      m_Translate(new Translate)
{
  setWindowTitle(QStringLiteral("极简翻译"));
  QVBoxLayout* pvLayout = new QVBoxLayout(this);
  pvLayout->addWidget(m_TextFrom);
  pvLayout->addWidget(m_TextTo);
  QHBoxLayout* phLayout = new QHBoxLayout;
  AddCombbox(phLayout);
  QPushButton* pButtonGet = new QPushButton(QStringLiteral("翻译"), this);
  phLayout->addWidget(pButtonGet);
  pvLayout->addLayout(phLayout);
  connect(pButtonGet, &QPushButton::clicked, this, std::bind(&TranslateDlg::GetResultData, this));

  m_TextFrom->installEventFilter(this);
  m_TextTo->installEventFilter(this);
  // QClipboard *clip = QApplication::clipboard();
  // clip->setText(m_TextTo->toPlainText());
}

TranslateDlg::~TranslateDlg() {
  delete m_Translate;
  delete m_TextFrom;
  delete m_TextTo;
}

void TranslateDlg::showEvent(QShowEvent *)
{
  QSize size = QGuiApplication::primaryScreen()->size();
  QSize mSize = frameSize();
  int x, y;
  y = ( mSize.height() + m_FormRect.bottom() > size.height() ) ? 
    m_FormRect.top() - mSize.height(): m_FormRect.bottom();
  if (((mSize.width() + m_FormRect.width()) >> 1) + m_FormRect.left() > size.width()) {
    x = size.width() - mSize.width();
  } else if ((m_FormRect.left() << 1) + m_FormRect.width() < mSize.width()) {
    x = 0;
  } else {
    x = m_FormRect.right() - ((m_FormRect.width() + mSize.width()) >> 1);
  }
  move(x, y);

  m_TextFrom->setFocus();
}


bool TranslateDlg::eventFilter(QObject *target, QEvent *event)
{
  static bool bShiftDown = false;
  if (target == m_TextFrom || target == m_TextTo)
  {
    if (event->type() == QEvent::KeyPress)
    {
      QKeyEvent *keyEvent = reinterpret_cast<QKeyEvent *>(event);
      switch (keyEvent->key()) {
        case Qt::Key_Escape:
          close();
          return true;
        case Qt::Key_Return:
        {
          QTextCursor&& pCursor = m_TextFrom->textCursor();
          if (pCursor.atBlockEnd() && !pCursor.atBlockStart()) {
            QChar c = m_TextFrom->document()->findBlockByLineNumber(pCursor.blockNumber()).text().back();
            if (c == QChar(' ')) {
              pCursor.deletePreviousChar();
              pCursor.insertHtml("<br/>");
              m_TextFrom->setTextCursor(pCursor);
              return true;
            }
          }
          GetResultData();
          return true;
        }
        case Qt::Key_AltGr:
          if (bShiftDown) {
            int i = m_BoxFrom->currentIndex();
            if (i == 0) {
              i = static_cast<int>(Translate::Lan::MAX) - 1;
            } else {
              --i;
            }
            m_Translate->SetFromLanguage(static_cast<Translate::Lan>(i));
            m_BoxFrom->setCurrentIndex(i);
          } else {
            int i = m_BoxFrom->currentIndex();
            if (++i == static_cast<int>(Translate::Lan::MAX)) {
              i = 0;
            }
            m_Translate->SetFromLanguage(static_cast<Translate::Lan>(i));
            m_BoxFrom->setCurrentIndex(i);
          }
          return true;
        case Qt::Key_Alt:
          if (bShiftDown) {
            int i = m_BoxTo->currentIndex() + 1;
            if (--i == 0) {
              i = static_cast<int>(Translate::Lan::MAX) - 1;
            }
            m_Translate->SetToLanguage(static_cast<Translate::Lan>(i));
            m_BoxTo->setCurrentIndex(i - 1);
          } else {
            int i = m_BoxTo->currentIndex() + 1;
            if (++i == static_cast<int>(Translate::Lan::MAX)) {
              i = 1;
            }
            m_Translate->SetToLanguage(static_cast<Translate::Lan>(i));
            m_BoxTo->setCurrentIndex(i - 1);
          }
          return true;
        case Qt::Key_Shift:
          bShiftDown = true;
          return true;
        default:
          break;
      }
    } else if (event->type() == QEvent::KeyRelease) {
      QKeyEvent *keyEvent = reinterpret_cast<QKeyEvent *>(event);
      if (keyEvent->key() == Qt::Key_Shift) {
        bShiftDown = false;
        return true;
      }
    }
  }
  return QWidget::eventFilter(target, event);
}

void TranslateDlg::Show(QRect rect, const QString& text) {
  m_FormRect = rect;
  m_TextFrom->setPlainText(text);
  m_TextTo->clear();
  show();
  activateWindow();
}

void TranslateDlg::GetResultData() {
  QByteArray array = m_TextFrom->toPlainText().toUtf8();
  auto result = m_Translate->GetResult(std::u8string(array.begin(), array.end()));
  m_TextTo->setPlainText(QString::fromUtf8(result.data(), result.size()));
}

void TranslateDlg::AddCombbox(QHBoxLayout* layout)
{

  m_BoxFrom = new QComboBox(this);
  QStringList lstFrom = { "自动检测", "中文简体", "中文繁体", "英语", "日语", "法语", "俄语" };
  m_BoxFrom->addItems(lstFrom);
  layout->addWidget(m_BoxFrom);

  m_BoxFrom->setCurrentIndex(static_cast<int>(m_Translate->GetCurFromLanguage()));
  connect(m_BoxFrom, &QComboBox::currentIndexChanged, this, [this](int index){
    m_Translate->SetFromLanguage(static_cast<Translate::Lan>(index));
  });

  QLabel *lable = new QLabel(this);
  lable->setText(QStringLiteral("→"));
  layout->addWidget(lable);

  m_BoxTo = new QComboBox(this);
  QStringList lstTo = { "中文简体", "中文繁体", "英语", "日语", "法语", "俄语" };
  m_BoxTo->addItems(lstTo);
  layout->addWidget(m_BoxTo);

  m_BoxTo->setCurrentIndex(static_cast<int>(m_Translate->GetCurToLanguage()) - 1);
  connect(m_BoxTo, &QComboBox::currentIndexChanged, this, [this](int index){
    m_Translate->SetToLanguage(static_cast<Translate::Lan>(index + 1));
  });
}
