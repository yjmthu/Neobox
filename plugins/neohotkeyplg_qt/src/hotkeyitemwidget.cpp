#include <hotkeyitemwidget.h>
#include <pluginmgr.h>
#include <pluginobject.h>
#include <yjson.h>
#include <glbobject.h>

#include <QEvent>
#include <QLabel>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

bool HotKeyItemWidget::isChecked = false;

HotKeyItemWidget::HotKeyItemWidget(QWidget* parent, const YJson& data)
    : QWidget(parent)
    , m_TargetPlugin(new QComboBox(this))
    , m_HotKey(new QPushButton(PluginObject::Utf82QString(data[u8"KeySequence"].getValueString()), this))
    , m_Enabled(new QCheckBox(this))
  {
    m_Enabled->setChecked(data[u8"Enabled"].isTrue());
    InitCombox(data[u8"Plugin"].getValueString());

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(new QLabel("插件名称", this));
    layout->addWidget(m_TargetPlugin);
    layout->addWidget(new QLabel("热键", this));
    layout->addWidget(m_HotKey);
    layout->addWidget(new QLabel("是否注册", this));
    layout->addWidget(m_Enabled);

    m_HotKey->setCheckable(true);
    m_HotKey->installEventFilter(this);
    connect(m_HotKey, &QPushButton::clicked, this, &HotKeyItemWidget::SetExlusive);
  }

HotKeyItemWidget::~HotKeyItemWidget()
{
  isChecked = false;
}

void HotKeyItemWidget::InitCombox(const std::u8string& plugin)
{
  int index = 0;
  for (int i = 0; auto [name, info]: mgr->GetPluginsInfo().getObject()) {
    m_TargetPlugin->addItem(PluginObject::Utf82QString(
          info[u8"FriendlyName"].getValueString()));
    if (name == plugin) index = i;
    ++i;
  }
  m_TargetPlugin->setCurrentIndex(index);
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
  auto const shortcut = m_HotKey->text();
  if (shortcut.isEmpty()) return false;
  auto friendlyName = PluginObject::QString2Utf8(m_TargetPlugin->currentText());
  for (auto& [name, info]: mgr->GetPluginsInfo().getObject()) {
    if (info[u8"FriendlyName"].getValueString() == friendlyName) {
      data.append(YJson::O {
        {u8"KeySequence", PluginObject::QString2Utf8(shortcut)},
        {u8"Enabled", m_Enabled->isChecked()},
        {u8"Plugin", name}
      });
      return true;
    }
  }
  return false;
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
