#include "translater.h"

#include <httplib.h>
#include <qxtglobalshortcut.h>
#include <sysapi.h>
#include <yjson.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <iostream>

Translater::Translater(QWidget* parent)
    : QWidget(parent),
      m_TextFrom(new QPlainTextEdit(this)),
      m_TextTo(new QPlainTextEdit(this)),
      m_BtnGroup(new QButtonGroup(this)) {
  SetupUi();
  m_TextFrom->installEventFilter(this);
  SetupKey();
}

Translater::~Translater() { delete m_Shortcut; }

void Translater::SetupUi() {
  setWindowTitle("极简翻译");
  setWindowIcon(QIcon(":/icons/speedbox.ico"));
  setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Dialog);
  QVBoxLayout* vbx = new QVBoxLayout(this);
  vbx->addWidget(m_TextFrom);
  vbx->addWidget(m_TextTo);
  QHBoxLayout* hbx = new QHBoxLayout;
  auto lst = {"自动", "汉英", "英汉"};
  int id = 0;
  m_BtnGroup->setExclusive(true);
  for (const auto& i : lst) {
    auto temp = new QCheckBox(this);
    temp->setText(i);
    m_BtnGroup->addButton(temp, id++);
    hbx->addWidget(temp);
  }
  qobject_cast<QCheckBox*>(m_BtnGroup->button(0))->setChecked(true);
  vbx->addLayout(hbx);
  for (id = 0; id < 3; ++id)
    connect(qobject_cast<QCheckBox*>(m_BtnGroup->button(id)),
            &QCheckBox::clicked, this,
            [this]() { GetReply(m_TextFrom->toPlainText()); });
}

void Translater::SetupKey() {
  m_Shortcut = new QxtGlobalShortcut;
  if (m_Shortcut->setShortcut(QKeySequence("Shift+Z"))) {
    connect(m_Shortcut, &QxtGlobalShortcut::activated, this,
            &Translater::IntelligentShow);
  } else {
    std::cout << "Can't register global shortcut.\n";
  }
}

void Translater::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Escape:
      hide();
      break;
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
      qobject_cast<QCheckBox*>(
          m_BtnGroup->button((m_BtnGroup->checkedId() + 1) % 3))
          ->setChecked(true);
      break;
    default:
      break;
  }
  event->accept();
}

void Translater::showEvent(QShowEvent* event) {
  m_TextFrom->setFocus();
  activateWindow();
  event->accept();
}

bool Translater::eventFilter(QObject* target, QEvent* event) {
  if (target == m_TextFrom) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* k = static_cast<QKeyEvent*>(event);
      if (k->key() == Qt::Key_Return) {
        QString text = m_TextFrom->toPlainText();
        auto cur = m_TextFrom->textCursor();
        if (text.isEmpty()) goto label;
        if (cur.position() >= QTextCursor::Start &&
            text.at(cur.position() - 1) == ' ') {
          cur.deletePreviousChar();
          cur.insertText("\n");
          return true;
        }
        GetReply(text);
        event->accept();
        return true;
      } else if (k->key() == Qt::Key_Delete) {
        m_TextFrom->clear();
        m_TextTo->clear();
        event->accept();
        return true;
      }
    }
  }
label:
  return QWidget::eventFilter(target, event);
}

void Translater::GetReply(const QString& text) {
  using namespace std::literals;
  static YJson param =
      YJson::O{{u8"doctype"sv, u8"json"sv}, {u8"i"sv, YJson::String}};
  httplib::Client clt("http://fanyi.youdao.com"s);
  QByteArray&& array = text.toUtf8();
  param[u8"i"sv].second.setText(array.cbegin(), array.cend());
  bool isChinese;
  if (m_BtnGroup->checkedId() == 0) {
    auto uNum = text.front().unicode();
    isChinese = uNum >= 0x4E00 && uNum <= 0x9FA5;
  } else {
    isChinese = m_BtnGroup->checkedId() == 1;
  }
  param[u8"type"sv].second = isChinese ? u8"ZH_CN2EN"sv : u8"EN2ZH_CN"sv;
  std::u8string url = param.urlEncode(u8"/translate?"sv);
  // std::cout << "http://fanyi.youdao.com"s << url << std::endl;
  auto res = clt.Get(reinterpret_cast<const char*>(url.c_str()));
  if (res->status != 200) {
    m_TextTo->setPlainText(QStringLiteral("网络开小差了~"));
    return;
  }
  auto json = YJson(res->body.cbegin(), res->body.cend());
  if (!json.isObject() || json.find(u8"errorCode"sv)->second != 0) {
    m_TextTo->setPlainText(QString::fromUtf8(
        res->body.data(), static_cast<int>(res->body.size())));
    return;
  }
  m_TextTo->clear();
  if (isChinese) {
    for (auto& i : json.find(u8"translateResult"sv)->second.getArray()) {
      std::u8string tempStr;
      for (auto& j : i.getArray()) {
        tempStr.append(j[u8"tgt"sv].second.getValueString());
        tempStr.push_back(' ');
      }
      if (!tempStr.empty()) tempStr.pop_back();
      m_TextTo->appendPlainText(
          QString::fromUtf8(reinterpret_cast<const char*>(tempStr.data()),
                            static_cast<int>(tempStr.size())));
    }
  } else {
    for (auto& i : json.find(u8"translateResult"sv)->second.getArray()) {
      std::u8string tempStr;
      for (auto& j : i.getArray()) {
        tempStr.append(j[u8"tgt"sv].second.getValueString());
      }
      m_TextTo->appendPlainText(
          QString::fromUtf8(reinterpret_cast<const char*>(tempStr.data()),
                            static_cast<int>(tempStr.size())));
    }
  }
}

void Translater::IntelligentShow() {
  if (isVisible()) {
    hide();
  } else {
    show();
  }
}

void Translater::Translate(const QString& text) {
  m_TextFrom->setPlainText(text);
  if (!isVisible()) show();
  GetReply(text);
}
