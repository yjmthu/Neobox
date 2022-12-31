#include <skinobject.h>

SkinObject::SkinObject(const TrafficInfo& trafficInfo, QWidget* parent)
  : m_TrafficInfo(trafficInfo)
  , m_Center(new QWidget(parent))
  , m_Units({"B", "K", "M", "G", "T", "P"})
{
  m_Center->move(0, 0);
}

SkinObject::~SkinObject() {
  delete m_Center;
}

void SkinObject::InitSize(QWidget* parent)
{
  auto const size = m_Center->frameSize();
  parent->setMinimumSize(size);
  parent->setMinimumSize(size);
  UpdateText();
  m_Center->show();
}