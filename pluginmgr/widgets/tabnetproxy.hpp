#ifndef TABNETPROXY_HPP
#define TABNETPROXY_HPP

#include <QWidget>

namespace Ui {
  class Form;
}

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
  void InitSignals();
private:
  class YJson& m_Settings;
  std::u8string& m_Username;
  std::u8string& m_Password;
  std::u8string& m_Domain;
  double& m_Port;
  double& m_Type;
  Ui::Form* ui;
};

#endif // TABNETPROXY_HPP
