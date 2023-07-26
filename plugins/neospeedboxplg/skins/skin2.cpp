#include <skinobject.h>

#include <ui_skin2.h>

#include <format>

#define Skin Skin2
#define newSkin newSkin2

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
  Ui_center2* m_Ui;
  const String m_NetFmt = L"{0:.0f} {1}/s";
};

Skin::Skin(QWidget* parent, const TrafficInfo& trafficInfo)
  : SkinObject(trafficInfo, parent)
  , m_Ui(new Ui_center2)
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

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesUp, m_NetFmt, m_Units);
  m_Ui->netUp->setText(QString::fromStdWString(buffer));

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesDown, m_NetFmt, m_Units);
  m_Ui->netDown->setText(QString::fromStdWString(buffer));

  buffer = std::format(L"{} ", static_cast<int>(m_TrafficInfo.memUsage * 100));
  m_Ui->memUse->setText(QString::fromStdWString(buffer));

  SetStyleSheet();
}

void Skin::SetStyleSheet()
{
  constexpr double delta = 0.0002f;
  static const QString fmt {
    "background-color:"
      "qconicalgradient("
        "cx:0.5, cy:0.5, angle:90, "
        "stop:0 #595c6b, "
        "stop:%1 #595c6b, "
        "stop:%2 transparent, "
        "stop:1 transparent"
      ");"
    "border-radius: 21px;"
  };
  double x1 = 1- m_TrafficInfo.memUsage;
  if (x1 > delta) x1 -= delta;
  double x2 = x1 + delta;
  m_Ui->memColorFrame->setStyleSheet(fmt.arg(x1, 0, 'f', 4).arg(x2, 0, 'f', 4));
}

#include <skinexport.cpp>
