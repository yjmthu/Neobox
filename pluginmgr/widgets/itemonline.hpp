#ifndef ITEMONLINE_HPP
#define ITEMONLINE_HPP

#include "itembase.hpp"
#include <yjson.h>

class ItemOnline: public ItemBase
{
  Q_OBJECT

public:
  explicit ItemOnline(std::u8string_view pluginName,
      const YJson& data, QWidget* parent);
  virtual ~ItemOnline();
public:
  void SetUninstalled();
  void SetUpdated();
protected:
  void DoFinished(FinishedType type, bool ok) override;
private:
  void InitLayout();
  void InitStatus();
  void InitConnect();
  YJson GetContent() const;
private:
  bool m_Installed;
  class QPushButton* m_BtnContrl;
  QLabel* m_LabelVersion;
};

#endif // ITEMONLINE_HPP
