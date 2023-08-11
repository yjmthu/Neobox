#pragma onece

#include <QWidget>
#include <QPainterPath>
#include <QTextOption>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Ui {
  class SmallForm;
}

class SmallForm: public QWidget
{
  // Q_OBJECT

protected:
  void paintEvent(QPaintEvent *event) override;
  void OnMouseWheel(QWheelEvent *event);
public:
  explicit SmallForm(class ColorConfig& settings);
  virtual ~SmallForm();
public:
  static void PickColor(ColorConfig& settings);
private:
  void SetColor(const QColor& color);
  void AutoPosition(const QPoint& point);
  void QuitHook(bool succeed);
  void ConnectAll(class ColorBack* back);
public:
  QColor m_Color;
  static SmallForm* m_Instance;
private:
  class ScreenFetch* const m_ScreenFetch;
  class ColorConfig& m_Settings;
  short m_ScaleTimes;
  class SquareForm* m_SquareForm;
  QPixmap m_BackPixMap;
  const QTextOption m_TextOption;
  QPainterPath m_ColorPath;
  const QFont m_TextFont;
};
