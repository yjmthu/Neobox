#include <skinobject.h>

#include <ui_skin4.h>

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
  const std::string m_NetFmt = "{0:.1f} {1}";
  const TrafficInfo::SpeedUnits m_Units = {
    "B/s", "KB/s", "MB/s", "GB/s", "TB/s", "PB/s"
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
  static std::string buffer;

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesUp, m_NetFmt, m_Units);
  m_Ui->netUp->setText(QString::fromUtf8(buffer.data(), buffer.size()));

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesDown, m_NetFmt, m_Units);
    m_Ui->netDown->setText(QString::fromUtf8(buffer.data(), buffer.size()));

  buffer = std::format("{}%", static_cast<int>(m_TrafficInfo.memUsage * 100));
  m_Ui->memUse->setText(QString::fromUtf8(buffer.data(), buffer.size()));

  buffer = std::format("{}%", static_cast<int>(m_TrafficInfo.cpuUsage * 100));
  m_Ui->cpuUse->setText(QString::fromUtf8(buffer.data(), buffer.size()));

  SetStyleSheet();
}

#include <skinexport.cpp>
