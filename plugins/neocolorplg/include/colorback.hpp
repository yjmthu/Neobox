#ifndef COLOR_BACK_HPP
#define COLOR_BACK_HPP

#include <QWidget>

class ColorBack: public QWidget
{
  Q_OBJECT

protected:
  void showEvent(QShowEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
public:
  explicit ColorBack(QPixmap pixmap);
  virtual ~ColorBack();
  QPoint& TransformPoint(QPoint& point);
public:
  QPixmap m_Pixmap;
  QImage m_Image;
signals:
  void InitShow();
  void MouseMove(QPoint, QColor);
  void MouseClick(QColor);
  void MouseWheel(QWheelEvent*);
};

#endif // COLOR_BACK_HPP