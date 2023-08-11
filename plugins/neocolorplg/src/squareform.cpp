#include <squareform.hpp>
#include <pluginmgr.h>
#include <screenfetch.hpp>

#include <QPainterPath>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>

#include <iostream>

constexpr int SquareForm::m_BorderWidth;

class ColorBigger: public QWidget
{
protected:
  void paintEvent(QPaintEvent *event) override {
    if (m_MouseGrid.isNull()) {
      return;
    }
    QPainter painter(this);
    painter.drawPixmap(m_MouseGrid, m_Pixmap);
    event->accept();
  }
private:
  QPixmap m_Pixmap;
  QPoint m_MouseGrid;
  short& m_ScalSize;
  float m_Radius = 0;
public:
  explicit ColorBigger(short& scalSize, QWidget* parent) : QWidget(parent), m_ScalSize(scalSize) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
  }
  void DrawGrids() {
    auto side = m_ScalSize * 5;
    m_Radius = side / 2.0;
    m_Pixmap = QPixmap(QSize(side, side));
    m_Pixmap.fill(Qt::transparent);
    QPainter painter(&m_Pixmap);
    QPointF center(m_Radius, m_Radius);
    QRadialGradient radialGradient(center, m_Radius, center);
    radialGradient.setColorAt(0.0, Qt::gray);
    radialGradient.setColorAt(1.0, Qt::transparent);

    painter.setPen(QPen(QBrush(radialGradient), 2.0));

    int varPos = 0;
    for (int i = 0; i != 5; ++i, (varPos += m_ScalSize)) {
      painter.drawLine(QPoint(varPos, 0), QPoint(varPos, m_Pixmap.height()));
      painter.drawLine(QPoint(0, varPos), QPoint(m_Pixmap.width(), varPos));
    }

    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(QRect(m_ScalSize * 2, m_ScalSize * 2, m_ScalSize, m_ScalSize));
  }
  void Update(const QPoint& point) {
    QPoint grid {
      point.x() / m_ScalSize * m_ScalSize - (m_ScalSize << 1),
      point.y() / m_ScalSize * m_ScalSize - (m_ScalSize << 1)
    };
    if (m_MouseGrid != grid) {
      m_MouseGrid = grid;
      // update();
      repaint();
    }
  }
  void Resize(const QSize& size) {
    setFixedSize(size);
    if (m_ScalSize < 9) {
      m_MouseGrid = QPoint();
    } else {
      DrawGrids();
    }
    
  }
};

SquareForm::SquareForm(const QPixmap& pix, const QPoint& pos, QWidget* parent)
  : ColorBack(pix)
  , m_Center(pos)
  , m_BaseSize(pix.size())
  , m_BaseScal(3)
  , m_ColorBigger(new ColorBigger(m_ScalSize, this))
{
  m_ColorBigger->move(m_BorderWidth, m_BorderWidth);
  SetScaleSize(1);
  if (m_Image.format() != QImage::Format_ARGB32 && m_Image.format() != QImage::Format_RGB32) {
    m_Image = m_Image.convertToFormat(QImage::Format_RGB32);
  }
}

SquareForm::~SquareForm()
{
}

void SquareForm::SetScaleSize(short times)
{
  // 1, 2, 3, 4
  if (times > 4 || times < 1) {
    return;
  }
  m_ScalSize = times * m_BaseScal; // 3, 6, 9, 12
  auto fixedSize = m_BaseSize * m_ScalSize;
  m_ColorBigger->Resize(fixedSize);
  fixedSize.rwidth() += m_BorderWidth << 1;
  fixedSize.rheight() += m_BorderWidth << 1;
  setFixedSize(fixedSize);
  DrawPicture();
  move(m_Center - frameGeometry().center() + pos());
}

void SquareForm::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.drawPixmap(0, 0, m_Pixmap);
}

void SquareForm::mouseMoveEvent(QMouseEvent *event)
{
  if (m_ScalSize < 9) return;
  m_ColorBigger->Update(event->pos() - QPoint(m_BorderWidth, m_BorderWidth));
  ColorBack::mouseMoveEvent(event);
}

void SquareForm::enterEvent(QEnterEvent *event) {
  auto p = qobject_cast<ScreenFetch*>(parent());
  p->setMouseTracking(false);
  setMouseTracking(true);
  ColorBack::enterEvent(event);
}

void SquareForm::leaveEvent(QEvent *event) {
  auto p = qobject_cast<ScreenFetch*>(parent());
  setMouseTracking(false);
  p->setMouseTracking(true);
  ColorBack::leaveEvent(event);
}

// void SquareForm::DrawBorder(QPainter& painter) const
// {
//   QPainterPath path;

//   auto size = this->frameSize();
//   auto rect = QRect(0, 0, m_BorderWidth << 1, m_BorderWidth << 1);
//   path.moveTo(m_BorderWidth, 0);
//   path.arcTo(rect, 90, 90);

// 	path.lineTo(0, size.height() - m_BorderWidth);

//   rect.moveTop(size.height() - m_BorderWidth * 2);
// 	path.arcTo(rect, 180, 90);

//   path.lineTo(size.width() - m_BorderWidth, size.height());
  
//   rect.moveLeft(size.width() - m_BorderWidth * 2);
//   path.arcTo(rect, 270, 90);

//   path.lineTo(size.width(), m_BorderWidth);

//   rect.moveTop(0);
//   path.arcTo(rect, 0, 90);

//   path.lineTo(m_BorderWidth, 0);

// 	painter.fillPath(path, QColor(20, 20, 20, 200));
// }

void SquareForm::DrawPicture()
{
  m_Pixmap = QPixmap(size());
  m_Pixmap.fill(Qt::transparent);

  QPainter painter(&m_Pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);
  QPainterPath path;
  path.addRoundedRect(rect(), m_BorderWidth, m_BorderWidth);

  painter.fillPath(path, QColor(20, 20, 20, 200));

  painter.setRenderHint(QPainter::Antialiasing, false);
  painter.translate(m_BorderWidth, m_BorderWidth);

  auto rect = QRect(0, 0, m_ScalSize, m_ScalSize);
  for (auto j=0; j!=m_Image.height(); ++j) {
    auto ptr = reinterpret_cast<QRgb*>(m_Image.scanLine(j));
    for (auto i=0; i!=m_Image.width(); ++i) {
      painter.fillRect(rect, QColor(*ptr++));
      rect.moveLeft(rect.left() + m_ScalSize);
    }
    rect.moveLeft(0);
    rect.moveTop(rect.top() + m_ScalSize);
  }
}

void SquareForm::showEvent(QShowEvent *event) 
{
  move(m_Center - frameGeometry().center() + pos());
  ColorBack::showEvent(event);
}
