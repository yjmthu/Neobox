#include <heightctrl.h>
#include <translatedlg.h>
#include <yjson.h>

#include <QWheelEvent>
#include <QPlainTextEdit>
#include <QTextEdit>

using namespace std::literals;

constexpr int HeightCtrl::m_MaxRatio;
constexpr int HeightCtrl::m_MinRatio;
constexpr int HeightCtrl::m_DefaultRatio;

HeightCtrl::HeightCtrl(NeoTranslateDlg* parent, YJson& setting)
  : QFrame(parent)
  , m_Baidu(setting[0].getValueDouble())
  , m_Youdao(setting[1].getValueDouble())
  , m_Source(parent->m_Translate->m_Source)
  , m_Changed(parent->m_LanPairChanged)
  , m_TextFrom(*parent->m_TextFrom)
  , m_TextTo(*parent->m_TextTo)
{
  if (m_Baidu > m_MaxRatio || m_Baidu < m_MinRatio) {
    m_Baidu = m_DefaultRatio;
  }
  if (m_Youdao > m_MaxRatio || m_Youdao < m_MinRatio) {
    m_Youdao = m_DefaultRatio; 
  }
  SetStyleSheet(m_Source == Translate::Baidu ? m_Baidu : m_Youdao);
}

void HeightCtrl::SetStyleSheet(double value)
{
  constexpr double delta = 0.0002f;
  value /= 360;
  if (value > delta) value -= delta;
  auto const style = std::format(
    "QFrame {{"
      "border-radius: 2px;"
      "border: 1px solid yellow;"
      "background-color: "
        "qlineargradient("
          "spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
          "stop:0 #07764c, "
          "stop:{0:.4f} #4c9b24, "
          "stop:{1:.4f} #FF5500, "
          "stop:1 #FF5500"
        ");"
    "}}"
    "QFrame::hover {{"
      "border-radius: 2px;"
      "border: 1px solid yellow;"
      "background-color: "
        "qlineargradient("
          "spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
          "stop:0 #00cc55, "
          "stop:{0:.4f} #00cc55, "
          "stop:{1:.4f} #FF0000, "
          "stop:1 #FF0000"
        ");"
    "}}",
    value, value + delta
  );
  setStyleSheet(QString::fromStdString(style));
}

void HeightCtrl::SetTextHeight(double value)
{
  value /= 360;
  auto const val = static_cast<int>(value * (m_TextFrom.height() + m_TextTo.height()));
  m_TextFrom.setMinimumHeight(val);
  m_TextFrom.setMaximumHeight(val);
  setToolTip(QString("%1%").arg(static_cast<int>(value * 100)));
}

void HeightCtrl::wheelEvent(QWheelEvent *event)
{
  auto const numDegrees = event->angleDelta() / 120;

  if (!numDegrees.isNull()) {
    auto numSteps = numDegrees.y();
    auto& value = (m_Source == Translate::Baidu ? (m_Baidu) : (m_Youdao));
    int val = static_cast<int>(value) - numSteps;
    if (numSteps && val > m_MinRatio && val < m_MaxRatio) {
      value = val;
      SetTextHeight(value);
      SetStyleSheet(value);
      m_Changed = true;
    }
  }

  event->accept();
}

