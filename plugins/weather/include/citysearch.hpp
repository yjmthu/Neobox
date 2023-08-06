#ifndef CITYSEARCH_HPP
#define CITYSEARCH_HPP

#include <QLineEdit>

class CitySearch: public QLineEdit
{
  Q_OBJECT

protected:
  void focusInEvent(QFocusEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;
public:
  explicit CitySearch(QWidget* parent);
  virtual ~CitySearch();
signals:
  void FocusIn();
  void FocusOut();
};

#endif // CITYSEARCH_HPP