#ifndef PLUGINCENTER_H
#define PLUGINCENTER_H

#include <QDialog>

class PluginCenter: public QDialog
{
public:
  explicit PluginCenter(class YJson& setting);
  virtual ~PluginCenter();
private:
  class YJson& m_Setting;
};

#endif // PLUGINCENTER_H
