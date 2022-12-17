#ifndef PLUGINITEM_H
#define PLUGINITEM_H

#include <QWidget>
#include <yjson.h>

class PluginItem: public QWidget
{
public:
  explicit PluginItem(std::u8string pluginName, const class YJson& data, QWidget* parent);
  virtual ~PluginItem();
private:
  void InitLayout();
  void InitStatus();
  void UpdateEnabled();
  bool DownloadPlugin();
  void Uninstall();
  void Install();
  void Upgrade();
private:
  class QVBoxLayout* m_MainLayout;
  class QHBoxLayout* m_SubLayout;
  class QPushButton* m_BtnUpgrade;
  class QPushButton* m_BtnUninstall;
  class QPushButton* m_BtnInstall;
  class QCheckBox* m_ChkEnable;
  const std::u8string m_PluginName;
  const YJson m_Data;

  bool m_Installed;
  bool m_CanUpdate;
  bool m_Enabled;
};

#endif // PLUGINITEM_H
