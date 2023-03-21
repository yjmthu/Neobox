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
  bool Checked() const;
protected:
  void paintEvent(QPaintEvent *) override;
private:
  int value;
  bool checked;
};

#endif // SWITCHBUTTON_HPP
