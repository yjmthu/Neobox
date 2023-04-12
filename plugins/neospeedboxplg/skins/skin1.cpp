#include <skinobject.h>

#include <ui_skin1.h>

#define Skin Skin1

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
  const std::string m_NetUpFmt = "↑ {0:.1f} {1}";
  const std::string m_NetDownFmt = "↓ {0:.1f} {1}";
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

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesUp, m_NetUpFmt, m_Units);
  m_Ui->netUp->setText(QString::fromUtf8(buffer.data(), buffer.size()));

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesDown, m_NetDownFmt, m_Units);
    m_Ui->netDown->setText(QString::fromUtf8(buffer.data(), buffer.size()));

  m_Ui->memUse->setText(QString::number(static_cast<int>(m_TrafficInfo.memUsage * 100), 10));

  SetStyleSheet();
}

#include <skinexport.cpp>
