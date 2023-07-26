#include <skinobject.h>

SkinObject::SkinObject(const TrafficInfo& trafficInfo, QWidget* parent)
  : m_TrafficInfo(trafficInfo)
  , m_Center(new QWidget(parent))
  , m_Units({L"B", L"K", L"M", L"G", L"T", L"P"})
{
  m_Center->move(0, 0);
}

SkinObject::~SkinObject() {
  delete m_Center;
}

void SkinObject::InitSize(QWidget* parent)
{
  auto const size = m_Center->frameSize();
  parent->setFixedSize(size);
  UpdateText();
  m_Center->show();
}