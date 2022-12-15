#include <hotkeyitemwidget.h>
#include <pluginmgr.h>
#include <pluginobject.h>
#include <yjson.h>
#include <neoapp.h>

#include <QEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>

bool HotKeyItemWidget::isChecked = false;

HotKeyItemWidget::HotKeyItemWidget(QWidget* parent, const std::u8string& name, const YJson& data)
    : QWidget(parent)
    , m_Description(new QLineEdit(PluginObject::Utf82QString(name), this))
    , m_HotKey(new QPushButton(PluginObject::Utf82QString(data[u8"KeySequence"].getValueString()), this))
    , m_Enabled(new QCheckBox(this))
  {
    setMinimumHeight(40);
    m_Enabled->setChecked(data[u8"Enabled"].isTrue());
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_Description);
    layout->addWidget(m_HotKey);
    layout->addWidget(m_Enabled);

    m_HotKey->setCheckable(true);
    m_HotKey->installEventFilter(this);
    connect(m_HotKey, &QPushButton::clicked, this, &HotKeyItemWidget::SetExlusive);
  }

HotKeyItemWidget::~HotKeyItemWidget()
{
  isChecked = false;
}

bool HotKeyItemWidget::eventFilter(QObject *obj, QEvent *event)
{
  // https://blog.csdn.net/sunflover454/article/details/50904815
  if(event->type() == QEvent::KeyPress && m_HotKey->isChecked())
  {
    auto keyevent = static_cast<QKeyEvent*>(event);
    int uKey = keyevent->key();
    Qt::Key key = static_cast<Qt::Key>(uKey);
    if(key == Qt::Key_unknown)
    {
      //nothing { unknown key }
    }

    if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt )
    {
      return false;
    }
    auto modifiers = keyevent->modifiers();
    if(modifiers & Qt::ShiftModifier)
      uKey += Qt::SHIFT;
    if(modifiers & Qt::ControlModifier)
      uKey += Qt::CTRL;
    if(modifiers & Qt::AltModifier)
      uKey += Qt::ALT;
    m_HotKey->setText(QKeySequence(uKey).toString(QKeySequence::NativeText));
    return true;
  }
  return QWidget::eventFilter(obj, event);
}

bool HotKeyItemWidget::WriteJson(YJson& data) const {
  auto const text = m_Description->text();
  if (text.isEmpty()) {
    return false;
  }
  auto const shortcut = m_HotKey->text();
  if (shortcut.isEmpty()) return false;
  data[PluginObject::QString2Utf8(text)] = YJson::O {
    {u8"KeySequence", PluginObject::QString2Utf8(shortcut)},
    {u8"Enabled", m_Enabled->isChecked()},
  };
  return true;
}

void HotKeyItemWidget::SetExlusive(bool on)
{
  if (on) {
    if (isChecked) {
      m_HotKey->setChecked(false);
    } else {
      isChecked = true;
    }
  } else {
    isChecked = false;
  }
}