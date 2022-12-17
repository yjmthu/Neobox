#ifndef PLUGINCENTER_H
#define PLUGINCENTER_H

#include <QDialog>

class PluginCenter: public QDialog
{
public:
  explicit PluginCenter(class YJson& setting);
  virtual ~PluginCenter();
public:
  static std::u8string GetRawUrl(const std::u8string& path);
private:
  bool GetPluginData();
private:
  class QVBoxLayout* m_MainLayout;
  class QListWidget* m_ListWidget;
  class YJson& m_Setting;
};

#endif // PLUGINCENTER_H
