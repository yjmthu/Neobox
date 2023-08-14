#include <neomsgdlg.hpp>

#include <QGuiApplication>
#include <QFrame>
#include <QTimer>
#include <QLabel>
#include <QHBoxLayout>
#include <QScreen>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

NeoMsgDlg::NeoMsgDlg(QWidget* parent)
  : QWidget(parent,
              Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
  , m_pOpacity(new QGraphicsOpacityEffect(this))
  , m_pAnimation(new QPropertyAnimation(this))
{
  InitWindowStyle();
  InitContent();
  InitAnimation();
}

NeoMsgDlg::~NeoMsgDlg()
{
}

void NeoMsgDlg::HandleShowMsg()
{
  QTimer::singleShot(1000, this, [this]() {
      m_Data.pop();
      if (m_Data.empty()) {
        m_pAnimation->start();
      } else {
        while (m_Data.size() > 3) m_Data.pop();
        m_pLabel->setText(m_Data.front());
        HandleShowMsg();
      }
    }
  );
}

void NeoMsgDlg::ShowMessage(const QString& text)
{
  if (!m_Data.empty()) {
    m_Data.push(text);
    return;
  }
  QMetaObject::invokeMethod(this, [this, text]() {
    m_Data.push(text);
    m_pLabel->setText(text);
    show();
    // m_pFrame->setWindowOpacity();
    HandleShowMsg();
  });
}

void NeoMsgDlg::InitWindowStyle()
{
  setWindowTitle("NeoboxMsgDlg");
  // setWindowFlags(Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground, true);

  setStyleSheet(
    "QFrame {"
      "background-color: #2870ea;"
      "border-radius: 5px;"
    "}"
    "QLabel {"
      "background-color: transparent;"
      "color: #ffffff;"
      "font-size: 15pt;"
    "}"
  );
}

void NeoMsgDlg::InitContent()
{
  m_pFrame = new QFrame(this);
  m_pLabel = new QLabel(m_pFrame);

  auto layout = new QHBoxLayout(m_pFrame);
  layout->addWidget(m_pLabel);
}

void NeoMsgDlg::InitAnimation()
{
  m_pFrame->setGraphicsEffect(m_pOpacity);
  m_pAnimation->setTargetObject(m_pOpacity);

  m_pAnimation->setPropertyName("opacity");
  m_pAnimation->setStartValue(1);
  m_pAnimation->setEndValue(0);
  m_pAnimation->setDuration(150);

  connect(m_pAnimation, &QPropertyAnimation::finished, this, &QWidget::hide);
}

void NeoMsgDlg::showEvent(QShowEvent *event)
{
  auto const screenSize = QGuiApplication::primaryScreen()->availableSize();
  m_pLabel->adjustSize();
  m_pFrame->adjustSize();
  this->adjustSize();

  const auto x = screenSize.width() - width(), y = screenSize.height() - height();
  move(x >> 1, y >> 1);

  m_pOpacity->setOpacity(1);

  QWidget::showEvent(event);
}
