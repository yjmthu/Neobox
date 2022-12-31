#ifndef SKINOBJECT_H
#define SKINOBJECT_H

#include <QWidget>

#include <trafficinfo.h>

class SkinObject
{
public:
  explicit SkinObject(const TrafficInfo& trafficInfo, QWidget* parent);
  virtual ~SkinObject();
  
public:
  virtual void UpdateText() = 0;

protected:
  void InitSize(QWidget* parent);

protected:
  const TrafficInfo& m_TrafficInfo;
  const TrafficInfo::SpeedUnits m_Units;
  QWidget* m_Center;
};

#endif // SKINOBJECT_H
