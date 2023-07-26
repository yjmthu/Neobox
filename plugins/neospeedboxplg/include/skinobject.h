#ifndef SKINOBJECT_H
#define SKINOBJECT_H

#include <QWidget>

#include <trafficinfo.h>


class SkinObject
{
protected:
  using String = TrafficInfo::FormatString;
  using SpeedUnits = TrafficInfo::SpeedUnits;
public:
  explicit SkinObject(const TrafficInfo& trafficInfo, QWidget* parent);
  virtual ~SkinObject();
  
public:
  virtual void UpdateText() = 0;
  // virtual void UiSettingDlg() = 0;

protected:
  void InitSize(QWidget* parent);

protected:
  const TrafficInfo& m_TrafficInfo;
  const SpeedUnits m_Units;
  QWidget* m_Center;
};

typedef SkinObject* SkinApi(QWidget*, const TrafficInfo&);

#endif // SKINOBJECT_H
