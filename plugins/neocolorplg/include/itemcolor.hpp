#ifndef ITEMCOLOR_HPP
#define ITEMCOLOR_HPP

#include <QWidget>

class ItemColor: public QWidget
{
  Q_OBJECT

public:
  explicit ItemColor(const QColor& color, QWidget* parent);
  virtual ~ItemColor();
public:
  void SetColor(const QColor& color);
  static ItemColor* GetCheckedItem();
public:
  QColor m_Color;
private:
  QWidget* m_CenterWidget;
};

#endif