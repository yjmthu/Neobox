#include <skinobject.h>

#include <ui_skin4.h>

#include <format>

#define Skin Skin4

class Skin: public SkinObject
{
public:
  explicit Skin(QWidget* parent, const TrafficInfo& trafficInfo);
  virtual ~Skin();

protected:
  void UpdateText() override;

private:
  void SetStyleSheet() {}

private:
  Ui_center* m_Ui;
  const String m_NetFmt = L"{0:.1f} {1}";
  const SpeedUnits m_Units = {
    L"B/s", L"KB/s", L"MB/s", L"GB/s", L"TB/s", L"PB/s"
  };
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

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesUp, m_NetFmt, m_Units);
  m_Ui->netUp->setText(QString::fromStdWString(buffer));

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesDown, m_NetFmt, m_Units);
  m_Ui->netDown->setText(QString::fromStdWString(buffer));

  buffer = std::format(L"{}%", static_cast<int>(m_TrafficInfo.memUsage * 100));
  m_Ui->memUse->setText(QString::fromStdWString(buffer));

  buffer = std::format(L"{}%", static_cast<int>(m_TrafficInfo.cpuUsage * 100));
  m_Ui->cpuUse->setText(QString::fromStdWString(buffer));

  SetStyleSheet();
}

#include <skinexport.cpp>
