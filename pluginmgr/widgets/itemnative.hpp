#ifndef ITEMNATIVE_HPP
#define ITEMNATIVE_HPP

#include "itembase.hpp"
#include <yjson/yjson.h>

class QPushButton;
class QCheckBox;

class ItemNative: public ItemBase
{
  Q_OBJECT

public:
  explicit ItemNative(std::u8string_view pluginName, const YJson& data, QWidget* parent);
  virtual ~ItemNative();
public:
  void SetUpdated();
  void UpdateStatus(const YJson& pluginsInfo);
  void GetContent(YJson& data) const;
protected:
  void DoFinished(FinishedType type, bool ok) override;
private:
  void InitLayout();
  void InitConnect();
private:
  QPushButton* m_BtnUpgrade;
  QPushButton* m_BtnUninstall;
  class SwitchButton* m_ChkEnable;
  QLabel* m_LabelNewVersion;
  QLabel* m_LabelOldVersion;
};

#endif // ITEMNATIVE_HPP
