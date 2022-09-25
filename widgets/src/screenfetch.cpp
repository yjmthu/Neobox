#include <screenfetch.h>

#include <QGuiApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QScreen>

ScreenFetch::ScreenFetch(QImage& image, QWidget* parent)
    : QWidget(parent),
      m_Image(image),
      m_bFirstClicked(0),
      m_bHaveCatchImage(false),
      m_PixMap(QGuiApplication::primaryScreen()->grabWindow()) {
  setWindowFlag(Qt::WindowStaysOnTopHint, true);
  setAttribute(Qt::WA_DeleteOnClose, true);
}

ScreenFetch::~ScreenFetch() {}

void ScreenFetch::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    if (!m_bFirstClicked++) {
      m_LeftTop = event->pos();
      setMouseTracking(true);
    } else {
      m_RectSize = event->pos() - m_LeftTop;
    }
  }
}

void ScreenFetch::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    if (m_bFirstClicked == 2) {
      if (hasMouseTracking())
        setMouseTracking(false);
      QImage temp = m_PixMap.toImage();
      double kx = static_cast<double>(temp.width()) / width();
      double ky = static_cast<double>(temp.height()) / height();
      m_Image = temp.copy(m_LeftTop.x() * kx, m_LeftTop.y() * ky,
                          m_RectSize.x() * kx, m_RectSize.y() * ky);
      m_bHaveCatchImage = true;
      close();
    }
    event->accept();
  } else {
    event->ignore();
  }
}

void ScreenFetch::mouseMoveEvent(QMouseEvent* event) {
  if (!m_bFirstClicked)
    return;
  if (event->buttons() == Qt::MiddleButton) {
    m_LeftTop = event->pos() - m_RectSize;
  } else {
    m_RectSize = event->pos() - m_LeftTop;
  }
  event->accept();
  update();
}

void ScreenFetch::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  painter.drawPixmap(0, 0, m_PixMap);
  painter.setPen(Qt::red);
  painter.drawRect(m_LeftTop.x(), m_LeftTop.y(), m_RectSize.x(),
                   m_RectSize.y());
  event->accept();
}

void ScreenFetch::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Escape:
      event->accept();
      close();
      break;
    default:
      event->ignore();
      break;
  }
}
