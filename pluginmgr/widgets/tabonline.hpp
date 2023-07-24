#ifndef TABONLINE_HPP
#define TABONLINE_HPP

#include <QWidget>

class TabOnline: public QWidget
{
  Q_OBJECT

public:
  explicit TabOnline(class PluginCenter* center);
  virtual ~TabOnline();
public:
  void UpdateItem(std::u8string_view pluginName, bool isUpdate);
public slots:
  bool UpdatePlugins();
private:
  void InitLayout();
  // void InitControls();
  void InitPlugins();
private:
  friend class ItemNative;
  class PluginCenter& m_PluginCenter;
  class QVBoxLayout* m_MainLayout;
  // class QHBoxLayout* m_ControlLayout;
  // class QPushButton* m_UpdateButton;
  class QListWidget* m_ListWidget;
};

#endif // TABONLINE_HPP
