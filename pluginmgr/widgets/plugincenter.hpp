#ifndef PLUGINCENTER_HPP
#define PLUGINCENTER_HPP

#include <widgetbase.hpp>

class PluginCenter: public WidgetBase
{
  Q_OBJECT 

public:
  explicit PluginCenter();
  virtual ~PluginCenter();
public:
  bool UpdatePluginData();
  std::optional<std::string> DownloadFile(std::u8string_view url);
private:
  void SetupUi();
  void InitConnect();
public:
  static const std::u8string m_RawUrl;
  static PluginCenter* m_Instance;
  class YJson* m_PluginData;
  using WidgetBase::AddScrollBar;
private:
  class YJson& m_Setting;
  class QVBoxLayout* m_MainLayout;
  class QTabWidget* m_TabWidget;
public:
  class TabNative* m_TabNative;
  class TabOnline* m_TabOnline;
private:
  class TabHotKey* m_TabHotKey;
  class TabNetProxy* m_TabNetProxy;
  class TabVersion* m_TabVersion;
};

#endif // PLUGINCENTER_HPP
