#ifndef TABNETPROXY_HPP
#define TABNETPROXY_HPP

#include <QWidget>

namespace Ui {
  class FormProxy;
}

class QButtonGroup;

class TabNetProxy: public QWidget
{
  Q_OBJECT
private slots:
  void SaveData();
  void InitData();
public:
  explicit TabNetProxy(QWidget* parent);
  ~TabNetProxy();
private:
  void InitLayout();
  void InitSignals();
private:
  Ui::FormProxy* ui;
  QButtonGroup* m_BtnGroup;
};

#endif // TABNETPROXY_HPP
