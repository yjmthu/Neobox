#include <screenfetch.h>

#include <QGuiApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QtSvg/QSvgRenderer>

ScreenFetch::ScreenFetch(QImage& image, QWidget* parent)
    : QWidget(parent),
      m_Image(image),
      m_bFirstClicked(0),
      m_bHaveCatchImage(false),
      m_PixMap(QGuiApplication::primaryScreen()->grabWindow()) {
  setWindowFlag(Qt::WindowStaysOnTopHint, true);
  setAttribute(Qt::WA_DeleteOnClose, true);
  SetCursor();
}

ScreenFetch::~ScreenFetch() {}

void ScreenFetch::SetCursor() {
	QSvgRenderer svgRender;
	svgRender.load(QStringLiteral(":/icons/cursor.svg"));
  QPixmap pixmap(16, 16);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  svgRender.render(&painter);
  QCursor cursor(pixmap, 5, 0);
  setCursor(cursor);
}

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
      if (m_RectSize.x() == 0 || m_RectSize.y() == 0) {
        m_Image = m_PixMap.toImage();
      } else {
        auto temp = m_PixMap.toImage();
        auto const kx = static_cast<float>(temp.width()) / width();
        auto const ky = static_cast<float>(temp.height()) / height();
        auto const x = std::min(m_LeftTop.x() + m_RectSize.x(), m_LeftTop.x());
        auto const y = std::min(m_LeftTop.y() + m_RectSize.y(), m_LeftTop.y());
        auto const w = std::abs(m_RectSize.x()), h = std::abs(m_RectSize.y());
        m_Image = temp.copy(x * kx, y * ky, w * kx, h * ky);
      }
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
