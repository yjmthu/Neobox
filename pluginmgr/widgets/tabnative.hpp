#ifndef TABNATIVE_HPP
#define TABNATIVE_HPP

#include <QWidget>

class TabNative: public QWidget
{
  Q_OBJECT

public:
  explicit TabNative(class PluginCenter* center);
  virtual ~TabNative();
public:
  void AddItem(std::u8string_view pluginName, const class YJson& data);
  void UpdateItem(std::u8string_view pluginName);
public slots:
  void UpdatePlugins();
  void UpgradeAllPlugins();
private slots:
  void SaveContent();
  void MoveUp();
  void MoveDown();
private:
  void InitLayout();
  void InitControls();
  void InitPlugins();
private:
  friend class ItemNative;
  class PluginCenter& m_PluginCenter;
  class QVBoxLayout* m_MainLayout;
  class QHBoxLayout* m_ControlLayout;
  class QListWidget* m_ListWidget;
};

#endif // TABNATIVE_HPP
