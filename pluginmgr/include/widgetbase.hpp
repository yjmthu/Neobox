#ifndef WIDGETBASE_HPP
#define WIDGETBASE_HPP

#include <QWidget>

class WidgetBase: public QWidget
{
  Q_OBJECT

protected:
  void showEvent(QShowEvent *event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
public:
  explicit WidgetBase(QWidget* parent);
  virtual ~WidgetBase();
protected:
  void AddCloseButton();
  void AddMinButton();
private:
  QPoint m_ConstPos;
  std::vector<class QPushButton*> m_Buttons;
};

#endif // WIDGETBASE_HPP
