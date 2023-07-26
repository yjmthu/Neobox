#include <skinobject.h>

#include <ui_skin6.h>

#include <format>

#define Skin Skin6

class Skin: public SkinObject
{
public:
  explicit Skin(QWidget* parent, const TrafficInfo& trafficInfo);
  virtual ~Skin();

protected:
  void UpdateText() override;

private:
  void SetStyleSheet();

private:
  Ui_center* m_Ui;
  const String m_NetUpFmt = L"↑ {0:.1f} {1}/s";
  const String m_NetDownFmt = L"↓ {0:.1f} {1}/s";
};

Skin::Skin(QWidget* parent, const TrafficInfo& trafficInfo)
  : SkinObject(trafficInfo, parent)
  , m_Ui(new Ui_center)
{
  m_Ui->setupUi(m_Center);
  InitSize(parent);
}

Skin::~Skin()
{
  delete m_Ui;
}

void Skin::UpdateText()
{
  static String buffer;

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesUp, m_NetUpFmt, m_Units);
  m_Ui->netUp->setText(QString::fromStdWString(buffer));

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesDown, m_NetDownFmt, m_Units);
  m_Ui->netDown->setText(QString::fromStdWString(buffer));

  buffer = std::format(L"{}%", static_cast<int>(m_TrafficInfo.memUsage * 100));
  m_Ui->memUse->setText(QString::fromStdWString(buffer));

  SetStyleSheet();
}

void Skin::SetStyleSheet() {
  constexpr double delta = 0.0002f;
  static const QString fmt {
    "background-color: "
      "qlineargradient("
        "spread:pad, x1:0.5, y1:0, x2:0.5, y2:1, "
        "stop:0 rgba(33, 203, 48, 200), "
        "stop:%1 #4c9b24, "
        "stop:%2 #51c55f, "
        "stop:1 rgba(179, 255, 57, 200)"
      ");"
    "border-radius: 3px;"
  };
  double x1 = 1- m_TrafficInfo.memUsage;
  if (x1 > delta) x1 -= delta;
  double x2 = x1 + delta;
  m_Ui->memColorFrame->setStyleSheet(fmt.arg(x1, 0, 'f', 4).arg(x2, 0, 'f', 4));
}

#include <skinexport.cpp>
