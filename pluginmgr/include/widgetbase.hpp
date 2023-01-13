#ifndef WIDGETBASE_HPP
#define WIDGETBASE_HPP

#include <QWidget>

class WidgetBase: public QWidget
{
  Q_OBJECT

protected:
  void showEvent(QShowEvent *event) override;
  bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
public:
  explicit WidgetBase(QWidget* parent, bool resizeAble=false);
  virtual ~WidgetBase();
protected:
  void AddCloseButton();
  void AddMinButton();
  void AddTitle(QString title);
  void SetShadowAround(QWidget* widget, int radius=20);
private:
  QPoint m_ConstPos;
  std::vector<class QPushButton*> m_Buttons;
  const bool m_ResizeAble;
};

#endif // WIDGETBASE_HPP
