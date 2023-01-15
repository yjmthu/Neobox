#ifndef WIDGETBASE_HPP
#define WIDGETBASE_HPP

#include <QWidget>

#if defined(MYSHAREDLIB_LIBRARY)
#  define MYSHAREDLIB_EXPORT Q_DECL_IMPORT
#else
#  define MYSHAREDLIB_EXPORT Q_DECL_EXPORT
#endif

class MYSHAREDLIB_EXPORT WidgetBase: public QWidget
{
  Q_OBJECT

protected:
  void showEvent(QShowEvent *event) override;
  bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
public:
  explicit WidgetBase(QWidget* parent, bool resizeAble=false, bool stayTop=false);
  virtual ~WidgetBase();
protected:
  virtual void SaveTopState(bool isTop) {}
  void AddTopButton();
  void AddCloseButton();
  void AddMinButton();
  void AddTitle(QString title);
  void SetShadowAround(QWidget* widget, int radius=20, QColor col=Qt::black, int dx=0, int dy=0);
private:
  QPoint m_ConstPos;
  std::vector<class QPushButton*> m_Buttons;
  const bool m_ResizeAble;
  bool m_StayTop;
};

#endif // WIDGETBASE_HPP
