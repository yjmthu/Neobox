#include <translate.h>

#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

Translate::Translate(QWidget* parent)
    : QDialog(parent),
      m_TextFrom(new QPlainTextEdit),
      m_TextTo(new QPlainTextEdit) {
  setWindowTitle(QStringLiteral("极简翻译"));
  QVBoxLayout* pvLayout = new QVBoxLayout(this);
  pvLayout->addWidget(m_TextFrom);
  pvLayout->addWidget(m_TextTo);
}

Translate::~Translate() {
  delete m_TextFrom;
  delete m_TextTo;
}

void Translate::Show(const QString& text) {
  m_TextFrom->setPlainText(text);
  exec();
}

void Translate::GetResultData() {}
