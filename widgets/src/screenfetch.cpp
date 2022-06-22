#include "screenfetch.h"

#include <leptonica/allheaders.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <iostream>

Pix* QImage2Pix(const QImage&& image) {
  Pix* pix;
  int width = image.width();
  int height = image.height();
  int depth = image.depth();
  pix = pixCreate(width, height, depth);
  if (image.isNull()) {
    std::cout << "image is null\n";
    return nullptr;
  }
  if (image.colorCount()) {
    QVector<QRgb> table = image.colorTable();

    PIXCMAP* map = pixcmapCreate(8);

    int n = table.size();
    for (int i = 0; i < n; i++) {
      pixcmapAddColor(map, qRed(table[i]), qGreen(table[i]), qBlue(table[i]));
    }
    pixSetColormap(pix, map);
  }
  int bytePerLine = image.bytesPerLine();
  l_uint32* start = pixGetData(pix);
  l_int32 wpld = pixGetWpl(pix);
  if (image.format() == QImage::Format_Mono ||
      image.format() == QImage::Format_Indexed8 ||
      image.format() == QImage::Format_RGB888) {
    for (int i = 0; i < height; i++) {
      const uchar* lines = image.scanLine(i);
      uchar* lined = (uchar*)(start + wpld * i);
      memcpy(lined, lines, static_cast<size_t>(bytePerLine));
    }
  } else if (image.format() == QImage::Format_RGB32 ||
             image.format() == QImage::Format_ARGB32) {
    std::cout << "QImage::Format_RGB32\n";
    for (int i = 0; i < image.height(); i++) {
      const QRgb* lines = (const QRgb*)image.scanLine(i);
      l_uint32* lined = start + wpld * i;
      for (int j = 0; j < width; j++) {
        uchar rval = qRed(lines[j]);
        uchar gval = qGreen(lines[j]);
        uchar bval = qBlue(lines[j]);
        l_uint32 pixel;
        composeRGBPixel(rval, gval, bval, &pixel);
        lined[j] = pixel;
      }
    }
  }
  return pix;
}

ScreenFetch::ScreenFetch()
    : QDialog(nullptr),
      m_Picture(nullptr),
      m_IsTrackingMouse(false),
      m_Pixmap(QGuiApplication::primaryScreen()->grabWindow(
          QApplication::desktop()->winId())),
      m_TextRect(0, 0, 0, 0) {
  setWindowFlags(Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setWindowState(windowState() ^ Qt::WindowFullScreen);
}

ScreenFetch::~ScreenFetch() {}

void ScreenFetch::paintEvent(QPaintEvent* event) {
  QPainter painter;
  painter.begin(this);
  QBrush brush;
  brush.setTexture(m_Pixmap);
  painter.setBrush(brush);
  painter.drawRect(rect());
  painter.fillRect(rect(), QColor(0, 0, 0, 150));
  painter.setCompositionMode(QPainter::CompositionMode_Clear);
  painter.eraseRect(m_TextRect);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.setPen(Qt::red);
  painter.drawRect(m_TextRect);
  painter.end();
  event->accept();
}

void ScreenFetch::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_K:
    case Qt::Key_Up:
      m_TextRect.setTop(m_TextRect.top() - 1);
      break;
    case Qt::Key_J:
    case Qt::Key_Down:
      m_TextRect.setTop(m_TextRect.top() + 1);
      break;
    case Qt::Key_H:
    case Qt::Key_Left:
      m_TextRect.setLeft(m_TextRect.left() - 1);
      break;
    case Qt::Key_L:
    case Qt::Key_Right:
      m_TextRect.setLeft(m_TextRect.left() + 1);
      break;
    case Qt::Key_Escape:
      if (m_IsTrackingMouse) {
        setMouseTracking(false);
      }
      close();
      break;
    default:
      break;
  }
  update();
  event->accept();
}

void ScreenFetch::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    if (m_IsTrackingMouse) {
      m_IsTrackingMouse = false;
      auto screen = QGuiApplication::primaryScreen();
      if (m_TextRect.width() && m_TextRect.height()) {
        double scale =
            (double)m_Pixmap.height() / (double)screen->geometry().height();
        m_Picture = QImage2Pix(m_Pixmap.toImage().copy(QRect(
            (double)m_TextRect.left() * scale, (double)m_TextRect.top() * scale,
            (double)m_TextRect.width() * scale,
            (double)m_TextRect.height() * scale)));
      } else {
        m_Picture = QImage2Pix(m_Pixmap.toImage());
      }
      close();
    } else {
      m_IsTrackingMouse = true;
      m_TextRect.setTopLeft(event->pos());
      m_TextRect.setBottomRight(event->pos());
    }
    setMouseTracking(m_IsTrackingMouse);
  }
  event->accept();
}

void ScreenFetch::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::MiddleButton) {
    m_TextRect.moveTo(event->pos() -
                      QPoint(m_TextRect.width(), m_TextRect.height()));
  } else {
    m_TextRect.setBottomRight(event->pos());
  }
  update();
  event->accept();
}
