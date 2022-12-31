#include <skinobject.h>

#include <ui_skin5.h>

#define Skin Skin5

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
  const std::string m_NetFmt = "{0:.0f} {1}";
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

  buffer = std::format("{:02}", static_cast<int>(m_TrafficInfo.memUsage * 100));
  m_Ui->memUse->setText(QString::fromUtf8(buffer.data(), buffer.size()));

  SetStyleSheet();
}

#include <skinexport.cpp>
