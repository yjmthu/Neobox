#ifndef PLUGINCENTER_HPP
#define PLUGINCENTER_HPP

#include <QDialog>

class PluginCenter: public QDialog
{
  Q_OBJECT 

signals:
  void DownloadFinished();
public:
  explicit PluginCenter(class YJson& setting);
  virtual ~PluginCenter();
public:
  bool UpdatePluginData();
private:
  void SetupUi();
  void InitConnect();
public:
  static const std::u8string m_RawUrl;
  static PluginCenter* m_Instance;
  class YJson* m_PluginData;
private:
  class YJson& m_Setting;
  class QVBoxLayout* m_MainLayout;
  class QTabWidget* m_TabWidget;
public:
  class TabNative* m_TabNative;
  class TabOnline* m_TabOnline;
private:
  class TabVersion* m_TabVersion;
};

#endif // PLUGINCENTER_HPP
