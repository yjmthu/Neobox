#include <colorback.hpp>

#include <QPainter>
#include <QPaintEvent>
#include <QScreen>

ColorBack::ColorBack(QPixmap pixmap)
  : QWidget(nullptr)
  , m_Pixmap(pixmap)
  , m_Image(pixmap.toImage())
{}

ColorBack::~ColorBack()
{}

QPoint& ColorBack::TransformPoint(QPoint& point) {
  auto scaleX = float(m_Image.width()) / width();
  auto scaleY = float(m_Image.height()) / height();
  point.rx() *= scaleX;
  point.ry() *= scaleY;
  return point;
}

void ColorBack::showEvent(QShowEvent *event) {
  emit InitShow();
  event->accept();
}

void ColorBack::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.drawPixmap(0, 0, m_Pixmap);
  event->accept();
}

void ColorBack::mouseMoveEvent(QMouseEvent *event) {
  auto pos = event->pos();
  emit MouseMove(event->pos() + this->pos(), m_Image.pixel(TransformPoint(pos)));
  event->accept();
}

void ColorBack::mouseReleaseEvent(QMouseEvent *event) {
  auto pos = event->pos();
  emit MouseClick(m_Image.pixel(TransformPoint(pos)));
}

void ColorBack::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    emit MouseClick(QColor());
  }
}

void ColorBack::wheelEvent(QWheelEvent *event) {
  emit MouseWheel(event);
}