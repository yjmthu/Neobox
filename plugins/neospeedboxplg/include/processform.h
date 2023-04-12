#ifndef PROCESSFORM_H
#define PROCESSFORM_H

#include <widgetbase.hpp>

class ProcessForm: public WidgetBase
{
protected:
  void showEvent(QShowEvent*);
public:
  explicit ProcessForm(class SpeedBox* box);
  ~ProcessForm();
public:
  void UpdateList();
  static QString FormatBytes(size_t size);
private:
  void SetupUi();
private:
  const int m_ProcessCount;
  class SpeedBox& m_SpeedBox;
  class ProcessHelper* m_ProcessHelper;
  QWidget* m_CenterWidget;
  class QLabel* m_Labels;
};

#endif // PROCESSFORM_H
