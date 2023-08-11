#include <skinobject.h>

#include <format>

#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>

using namespace std::literals;

#define Skin Skin

class Skin: public SkinObject
{
public:
  explicit Skin(QWidget* parent, const TrafficInfo& trafficInfo);
  virtual ~Skin();

protected:
  void UpdateText() override;

private:
  void SetupUi();
  void SetStyleSheet();

private:
  const String m_UpFmt { L"↑ {0:.0f} {1}"s };
  const String m_DownFmt { L"↓ {0:.0f} {1}"s };
  QLabel* m_NetUp;
  QLabel* m_NetDown;
};

Skin::Skin(QWidget* parent, const TrafficInfo& trafficInfo)
  : SkinObject(trafficInfo, parent)
  , m_NetUp(new QLabel(m_Center))
  , m_NetDown(new QLabel(m_Center))
{
  SetupUi();
  SetStyleSheet();
  InitSize(parent);
}

Skin::~Skin()
{
  delete m_NetDown;
  delete m_NetUp;
}

void Skin::UpdateText()
{
  static std::wstring buffer;

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesUp, m_UpFmt, m_Units);
  m_NetUp->setText(QString::fromStdWString(buffer));

  buffer = m_TrafficInfo.FormatSpeed(m_TrafficInfo.bytesDown, m_DownFmt, m_Units);
  m_NetDown->setText(QString::fromStdWString(buffer));

  buffer = std::format(L"CPU {:>3}%  RAM {:>3}%",
      static_cast<int>(m_TrafficInfo.cpuUsage * 100),
      static_cast<int>(m_TrafficInfo.memUsage * 100)
  );

  m_Center->setToolTip(QString::fromStdWString(buffer));
}

void Skin::SetupUi()
{
  auto const mainLayout = new QHBoxLayout(m_Center);
  mainLayout->addWidget(m_NetUp);
  mainLayout->addWidget(m_NetDown);

  constexpr int textWidth = 50;
  m_NetUp->setFixedWidth(textWidth);
  m_NetDown->setFixedWidth(textWidth);

  m_Center->setFixedSize(130, 30);
  m_Center->setCursor(Qt::PointingHandCursor);
}

void Skin::SetStyleSheet()
{
  m_Center->setStyleSheet(
    "background-color: rgba(20, 20, 20, 150);"
    "border: solid 2px gray;"
    "border-radius: 4px;"
  );
  m_NetUp->setStyleSheet(
    "color: white;"
    "background-color: transparent;"
  );
  m_NetDown->setStyleSheet(
    "color: white;"
    "background-color: transparent;"
  );
}

#include <skinexport.cpp>
