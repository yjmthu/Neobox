#ifndef SWITCHBUTTON_HPP
#define SWITCHBUTTON_HPP

#include <QWidget>

class SwitchButton: public QWidget
{
  Q_OBJECT

public:
  explicit SwitchButton(QWidget* parent);
  ~SwitchButton();
public:
  void SetChecked(bool checked, bool animate=false);
  bool IsChecked() const;
  void Toggle(bool animate=false);
signals:
  void Clicked(bool);
protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
private:
  class QVariantAnimation *m_Animation;
  int value;
  bool checked;
};

#endif // SWITCHBUTTON_HPP
