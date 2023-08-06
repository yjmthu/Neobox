#include <citysearch.hpp>

void CitySearch::focusInEvent(QFocusEvent *event)
{
  emit FocusIn();
  QLineEdit::focusInEvent(event);
}

void CitySearch::focusOutEvent(QFocusEvent *event)
{
  emit FocusOut();
  QLineEdit::focusOutEvent(event);
}

CitySearch::CitySearch(QWidget* parent)
  : QLineEdit(parent)
{
  setFixedWidth(150);
  setPlaceholderText(QStringLiteral("输入城市、乡镇"));
}

CitySearch::~CitySearch()
{}
