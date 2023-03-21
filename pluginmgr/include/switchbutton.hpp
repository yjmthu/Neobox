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
  void SetChecked(bool checked);
  bool IsChecked() const;
  void Toggle();
signals:
  void Clicked(bool);
protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
private:
  int value;
  bool checked;
};

#endif // SWITCHBUTTON_HPP
