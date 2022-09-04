#ifndef SCREENFETCH_H
#define SCREENFETCH_H

#include <QWidget>

class ScreenFetch: public QWidget
{
 protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
 public:
  explicit ScreenFetch(QImage& image, QWidget* parent=nullptr);
  ~ScreenFetch();
 private:
  QImage& m_Image;
  uint8_t m_bFirstClicked = 0;
  QPoint m_LeftTop, m_RectSize;
  const QPixmap m_PixMap;
};

#endif
