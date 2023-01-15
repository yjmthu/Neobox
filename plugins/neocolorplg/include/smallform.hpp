#ifndef SMALLFORM_HPP
#define SMALLFORM_HPP

#include <QWidget>

namespace Ui {
  class SmallForm;
}

class SmallForm: public QWidget
{
  // Q_OBJECT

protected:
  void showEvent(QShowEvent *event) override;
public:
  explicit SmallForm(QWidget* parent=nullptr);
  virtual ~SmallForm();
public:
  void GetScreenColor(int x, int y);
  void AutoPosition();
  void MouseWheel(short value);
private:
  void TransformPoint();
  void SetColor(const QColor& color);
public:
  QPoint m_Position;
  QColor m_Color;
private:
  Ui::SmallForm* ui;
  QScreen* m_Screen;
  short m_ScaleTimes;
  class SquareForm* m_SquareForm;
};

#endif // SMALLFORM_HPP