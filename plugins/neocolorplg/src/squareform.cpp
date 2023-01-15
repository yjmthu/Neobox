#include <squareform.hpp>
#include <pluginmgr.h>

#include <QPainterPath>
#include <QPainter>
#include <QMouseEvent>

constexpr int SquareForm::m_BorderWidth;

SquareForm::SquareForm(const QPixmap& pix, const QPoint& pos, QWidget* parent)
  : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
  , m_Center(pos)
  , m_BaseSize(pix.size())
  , m_BaseScal(3)
  , m_PixMap(pix)
{
  setWindowTitle("Neobox-颜色放大器");
  setAttribute(Qt::WA_TranslucentBackground);
  SetScaleSize(1);
  // m_PixMaps[m_ScalSize] = pix;
  setMouseTracking(true);
}

SquareForm::~SquareForm()
{
  setMouseTracking(false);
}

void SquareForm::SetScaleSize(short times)
{
  // 1, 2, 3, 4
  if (times > 4 || times < 1) {
    return;
  }
  m_ScalSize = times * m_BaseScal; // 3, 6, 9, 12
  auto fixedSize = m_BaseSize * m_ScalSize;
  fixedSize.rwidth() += 2 * m_BorderWidth;
  fixedSize.rheight() += 2 * m_BorderWidth;
  setFixedSize(fixedSize);
  move(m_Center - frameGeometry().center() + pos());
}

void SquareForm::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  DrawBorder(painter);
  painter.setRenderHint(QPainter::Antialiasing, false);

  painter.drawPixmap(QRect(QPoint(m_BorderWidth, m_BorderWidth), m_BaseSize * m_ScalSize), m_PixMap);
  if (m_ScalSize >= 9) {
    DrawMouseArea(painter);
  }
  QWidget::paintEvent(event);
}

void SquareForm::DrawBorder(QPainter& painter) const
{
  QPainterPath path;

  auto size = this->frameSize();
  auto rect = QRect(0, 0, m_BorderWidth * 2, m_BorderWidth * 2);
  path.moveTo(m_BorderWidth, 0);
  path.arcTo(rect, 90, 90);

	path.lineTo(0, size.height() - m_BorderWidth);

  rect.moveTop(size.height() - m_BorderWidth * 2);
	path.arcTo(rect, 180, 90);

  path.lineTo(size.width() - m_BorderWidth, size.height());
  
  rect.moveLeft(size.width() - m_BorderWidth * 2);
  path.arcTo(rect, 270, 90);

  path.lineTo(size.width(), m_BorderWidth);

  rect.moveTop(0);
  path.arcTo(rect, 0, 90);

  path.lineTo(m_BorderWidth, 0);

	painter.fillPath(path, QColor(20, 20, 20, 200));
}

void SquareForm::DrawMouseArea(QPainter& painter) const
{
  if (m_MouseGrid.isNull())
    return;
  
  // QPoint grid = (m_MouseGrid - QPoint(m_BorderWidth, m_BorderWidth)) / m_ScalSize;
  
  painter.save();
  painter.translate(m_BorderWidth, m_BorderWidth);
  auto radius = m_ScalSize << 1;
  int xMin = std::max(0, m_MouseGrid.x() - radius);
  int xMax = std::min(width() - (m_BorderWidth << 1), m_MouseGrid.x() + radius);
  int yMin = std::max(0, m_MouseGrid.y() - radius);
  int yMax = std::min(height() - (m_BorderWidth << 1), m_MouseGrid.y() + radius);
  
  QRadialGradient radialGradient(m_MouseGrid, radius, m_MouseGrid);
  radialGradient.setColorAt(0, Qt::gray);
  radialGradient.setColorAt(1, Qt::transparent);
  painter.setPen(QPen(QBrush(radialGradient), 1));

  for (int x = xMin / m_ScalSize * m_ScalSize; x <= xMax; x += m_ScalSize) {
    painter.drawLine(QPoint(x, yMin), QPoint(x, yMax));
  }

  for (int y = yMin / m_ScalSize * m_ScalSize; y <= yMax; y += m_ScalSize) {
    painter.drawLine(QPoint(xMin, y), QPoint(xMax, y));
  }

  painter.setPen(QPen(QColor(Qt::white)));
  painter.drawRect(QRect((m_MouseGrid - QPoint(m_ScalSize, m_ScalSize) / 2) / m_ScalSize * m_ScalSize, QSize(m_ScalSize, m_ScalSize)));
  // painter.restore();
}

void SquareForm::showEvent(QShowEvent *event) 
{
  move(m_Center - frameGeometry().center() + pos());
  QWidget::showEvent(event);
}

void SquareForm::mouseMoveEvent(QMouseEvent *event)
{
  m_MouseGrid = event->pos() - QPoint(m_BorderWidth, m_BorderWidth);
  update();
}
